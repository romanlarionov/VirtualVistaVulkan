
#ifndef VIRTUALVISTA_TEXTUREMANAGER_H
#define VIRTUALVISTA_TEXTUREMANAGER_H

#include <vector>
#include <string>

#include "gli/gli.hpp"

#include "VulkanSampler.h"
#include "VulkanDevice.h"
#include "VulkanImageView.h"

namespace vv
{
    struct SampledTexture
    {
        VulkanImage *image = nullptr;
        VulkanImageView * image_view = nullptr;
        VulkanSampler *sampler = nullptr;
    };

	class TextureManager
	{
	public:
		TextureManager();
		~TextureManager();

        /*
         * Creates a texture importing + management system. This class holds ownership over all texture data loaded.
         * This manager will only perform loading after verifying that it isn't already loaded.
         */
		void create(VulkanDevice *device);

        /*
         *
         */
		void shutDown();

        /*
         * Loads a texture from file.
         *
         * note: only png, jpeg, dds, and ktx file formats are supported for now.
         */
        SampledTexture* load2DImage(std::string path, std::string name, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                                    bool create_mip_levels = true);

        /*
         * Loads a provided cube map from file.
         *
         * note: only dds and ktx file formats are supported for now.
         */
        SampledTexture* loadCubeMap(std::string path, std::string name, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                                    bool create_mip_levels = true);

	private:
		VulkanDevice *_device;
        std::string _texture_directory;

        // Stores constructed textures/cube maps this class creates and is in current use.
        std::unordered_map<std::string, SampledTexture *> _loaded_textures;

        // Stores raw texture data on host memory.
        std::unordered_map<std::string, unsigned char *> _ldr_texture_array_data_cache;
        std::unordered_map<std::string, float *> _hdr_texture_array_data_cache;

        std::unordered_map<gli::format, VkFormat> _gliToVulkanFormat =
		{
			{ gli::FORMAT_RGBA8_UNORM_PACK8, VK_FORMAT_R8G8B8A8_UNORM },
			{ gli::FORMAT_RGBA32_SFLOAT_PACK32, VK_FORMAT_R32G32B32A32_SFLOAT },
			{ gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, VK_FORMAT_BC3_UNORM_BLOCK },
			{ gli::FORMAT_RG32_SFLOAT_PACK32, VK_FORMAT_R32G32_SFLOAT },
			{ gli::FORMAT_RGB8_UNORM_PACK8, VK_FORMAT_R8G8B8_UNORM }
		};

        /*
         * Generalized function to abstract loading of different texture types.
         */
        SampledTexture* loadTexture(void *data, VkDeviceSize size_in_bytes, VkExtent3D extent, VkFormat format,
            VkImageCreateFlags flags, uint32_t mip_levels, uint32_t array_layers, VkImageViewType image_view_type);
	};
}

#endif // VIRTUALVISTA_TEXTUREMANAGER_H