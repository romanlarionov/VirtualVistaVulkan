
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Settings.h"
#include "TextureManager.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	TextureManager::TextureManager()
	{
	}


	TextureManager::~TextureManager()
	{
	}


	void TextureManager::create(VulkanDevice *device)
	{
        _device = device;
        _texture_directory = Settings::inst()->getTextureDirectory();

        // load dummy texture
        SampledTexture *dummy_texture = load2DImage(_texture_directory, "dummy.png", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        VV_ASSERT(dummy_texture, "Dummy texture couldn't be loaded. Do you move something?");
	}


	void TextureManager::shutDown()
	{
        for (auto &t : _loaded_textures)
        {
            t.second->image->shutDown(); delete t.second->image;
            t.second->image_view->shutDown(); delete t.second->image_view;
            t.second->sampler->shutDown(); delete t.second->sampler;
        }

        for (auto &d : _ldr_texture_array_data_cache)
            stbi_image_free(d.second);
	}


    SampledTexture* TextureManager::load2DImage(std::string path, std::string name, VkFormat format, VkImageUsageFlagBits usage)
    {
        std::string file_type = name.substr(name.find_first_of('.') + 1);

        // check if geometry has already been loaded
        if (_loaded_textures.count(path + name) > 0)
            return _loaded_textures[path + name];

        if (name == "")
            return _loaded_textures[_texture_directory + "dummy.png"];

        if (file_type == "png" || file_type == "jpg")
        {
		    int stb_format = (format == VK_FORMAT_R8G8B8A8_UNORM) ? STBI_rgb_alpha : 0; // todo: figure out how other formats play with stb
            int width, height, depth, channels;

		    // loads the image into a 1d array w/ 4 byte channel elements.
		    unsigned char *texels = stbi_load((path + name).c_str(), &width, &height, &channels, stb_format);

            if (!texels)
            {
                VV_ASSERT(false, "Could not load texture at location: " + path + name);
                return _loaded_textures[_texture_directory + "dummy.png"];
            }

            _ldr_texture_array_data_cache[path + name] = texels;

		    uint32_t size_in_bytes = width * height * 4; // todo: test if channels is set here
            VkExtent3D extent = { width, height, 1 };

            // todo: add runtime mip map generation
            VulkanImage *image = new VulkanImage();
            image->create(_device, extent, format, usage, VK_IMAGE_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1,
                          VK_IMAGE_LAYOUT_PREINITIALIZED, VK_SAMPLE_COUNT_1_BIT);
            image->updateAndTransfer(texels, size_in_bytes);

            VulkanImageView *image_view = new VulkanImageView();
            image_view->create(_device, image, VK_IMAGE_VIEW_TYPE_2D);

            // todo: fix sampler creation. I have it hardcoded atm.
            VulkanSampler *sampler = new VulkanSampler();
            sampler->create(_device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, true, 16, VK_SAMPLER_MIPMAP_MODE_LINEAR,
                0.f, 0.f, 0.f, false);

            SampledTexture *texture = new SampledTexture();
            texture->image = image;
            texture->image_view = image_view;
            texture->sampler = sampler;

            _loaded_textures[path + name] = texture;
            return texture;
        }
        else if (file_type == "dds" || file_type == "ktx")
        {

        }
        
        VV_ASSERT(false, "File type: " + file_type + " not supported");
        return nullptr;
    }


    SampledTexture* TextureManager::loadCubeMap(std::string path, std::string name, VkFormat format, VkImageUsageFlagBits usage)
    {
        return nullptr;
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
