
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>
#include <cmath>

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
        m_device = device;
        m_texture_directory = Settings::inst()->getTextureDirectory();

        // load dummy texture
        SampledTexture *dummy_texture = load2DImage(m_texture_directory, "dummy.png", VK_FORMAT_R8G8B8A8_UNORM, false);
        VV_ASSERT(dummy_texture, "Dummy texture couldn't be loaded. Do you move something?");
	}


	void TextureManager::shutDown()
	{
        for (auto &t : m_loaded_textures)
        {
            t.second->image->shutDown(); delete t.second->image;
            t.second->image_view->shutDown(); delete t.second->image_view;
            t.second->sampler->shutDown(); delete t.second->sampler;
        }

        for (auto &d : m_ldr_texture_array_data_cache)
            stbi_image_free(d.second);
	}


    SampledTexture* TextureManager::load2DImage(std::string path, std::string name, VkFormat format, bool create_mip_levels)
    {
        std::string file_type = name.substr(name.find_first_of('.') + 1);

        // check if geometry has already been loaded
        if (m_loaded_textures.count(path + name) > 0)
            return m_loaded_textures[path + name];

        if (name == "")
            return m_loaded_textures[m_texture_directory + "dummy.png"];

        if (file_type == "png" || file_type == "jpg")
        {
		    int stb_format = (format == VK_FORMAT_R8G8B8A8_UNORM) ? STBI_rgb_alpha : 0; // todo: figure out how other formats play with stb
            int width, height, depth, channels;

		    // loads the image into a 1d array w/ 4 byte channel elements.
		    unsigned char *texels = stbi_load((path + name).c_str(), &width, &height, &channels, stb_format);

            if (!texels)
            {
                VV_ASSERT(false, "Could not load texture at location: " + path + name);
                return m_loaded_textures[m_texture_directory + "dummy.png"];
            }

            m_ldr_texture_array_data_cache[path + name] = texels;

            uint32_t size = width * height * 4;
            VkExtent3D extent = {};
            extent.width = static_cast<uint32_t>(width);
			extent.height = static_cast<uint32_t>(height);
            extent.depth = 1;
            uint32_t mip_levels = (create_mip_levels) ? std::floor(std::log2(std::max(extent.width, extent.height))) + 1 : 1;

            m_loaded_textures[path + name] = loadTexture(texels, size, extent, format, 0, 1, 1, VK_IMAGE_VIEW_TYPE_2D);
            return m_loaded_textures[path + name];
        }
        else if (file_type == "dds" || file_type == "ktx")
        {
            gli::texture_cube texels(gli::load((path + name).c_str()));
            m_hdr_texture_array_data_cache[path + name] = static_cast<float *>(texels.data());

            // todo: should implement a fallback
            if (texels.empty())
                throw std::runtime_error("HDR texture could not be loaded." + path + name);

            VkFormat fmt = m_gli_to_vulkan_format_map.at(texels.format());

            VkExtent3D extent = {};
            extent.width = static_cast<uint32_t>(texels.extent().x);
			extent.height = static_cast<uint32_t>(texels.extent().y);
            extent.depth = 1;
            uint32_t mip_levels = (create_mip_levels) ? static_cast<uint32_t>(texels.levels()) : 1;

            m_loaded_textures[path + name] = loadTexture(texels.data(), texels.size(), extent, fmt,
                                            0, mip_levels, 1, VK_IMAGE_VIEW_TYPE_2D);

            return m_loaded_textures[path + name];
        }
                
        VV_ASSERT(false, "File type: " + file_type + " not supported");
        return nullptr;
    }


    SampledTexture* TextureManager::loadCubeMap(std::string path, std::string name, VkFormat format, bool create_mip_levels)
    {
        std::string file_type = name.substr(name.find_first_of('.') + 1);

        // check if geometry has already been loaded
        if (m_loaded_textures.count(path + name) > 0)
            return m_loaded_textures[path + name];

        if (file_type == "dds" || file_type == "ktx")
        {
            gli::texture_cube cube(gli::load((path + name).c_str()));

            // todo: should implement a fallback for cube maps
            if (cube.empty())
                throw std::runtime_error("Cube map could not be loaded." + path + name);

            VkFormat fmt = m_gli_to_vulkan_format_map.at(cube.format());

            VkExtent3D extent = {};
            extent.width = static_cast<uint32_t>(cube.extent().x);
			extent.height = static_cast<uint32_t>(cube.extent().y);
            extent.depth = 1;
            uint32_t mip_levels = static_cast<uint32_t>(cube.levels());

            m_loaded_textures[path + name] = loadTexture(cube.data(), cube.size(), extent, fmt,
                                            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, mip_levels, 6, VK_IMAGE_VIEW_TYPE_CUBE);

            return m_loaded_textures[path + name];
        }

        return nullptr;
    }


    SampledTexture* TextureManager::loadTexture(void *data, VkDeviceSize size_in_bytes, VkExtent3D extent, VkFormat format,
        VkImageCreateFlags flags, uint32_t mip_levels, uint32_t array_layers, VkImageViewType image_view_type)
    {
        SampledTexture *texture = new SampledTexture();

        texture->image = new VulkanImage();
        texture->image->create(m_device, extent, format, VK_IMAGE_TYPE_2D, flags, VK_IMAGE_ASPECT_COLOR_BIT, 
                      mip_levels, array_layers, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_SAMPLE_COUNT_1_BIT);
        texture->image->updateAndTransfer(data, size_in_bytes);
        
        // create image views for each mip level
        texture->image_view = new VulkanImageView();
        texture->image_view->create(m_device, texture->image, image_view_type, 0);
        
        // todo: fix sampler creation. I have it hardcoded atm.
        texture->sampler = new VulkanSampler();
        texture->sampler->create(m_device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true, 16, VK_SAMPLER_MIPMAP_MODE_LINEAR,
            0.f, 0.f, float(mip_levels - 1), false);

        return texture;
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
