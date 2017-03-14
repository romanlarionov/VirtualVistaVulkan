
#ifndef VIRTUALVISTA_TEXTUREMANAGER_H
#define VIRTUALVISTA_TEXTUREMANAGER_H

#include <vector>
#include <string>

#include "VulkanSampler.h"
#include "VulkanDevice.h"
#include "VulkanImageView.h"

namespace vv
{
    struct SampledTexture
    {
        VulkanImageView *image_view = nullptr;
        VulkanImage *image = nullptr;
        VulkanSampler *sampler = nullptr;
    };

	class TextureManager
	{
	public:
		TextureManager();
		~TextureManager();

        /*
         * 
         */
		void create(VulkanDevice *device);

        /*
         *
         */
		void shutDown();

        /*
         * Loads a texture from file.
         *
         * note: only png, dds, and ktx file formats are supported for now.
         */
        SampledTexture* load2DImage(std::string path, std::string name, VkFormat format, VkImageUsageFlagBits usage);

        /*
         * Loads a provided cube map from file.
         */
        SampledTexture* loadCubeMap(std::string path, std::string name, VkFormat format, VkImageUsageFlagBits usage);

	private:
		VulkanDevice *_device;
        std::string _texture_directory;

        // Stores constructed textures/cube maps this class creates and is in current use.
        std::unordered_map<std::string, SampledTexture *> _loaded_textures;

        // Stores raw texture data on host memory.
        std::unordered_map<std::string, unsigned char *> _ldr_texture_array_data_cache;
        std::unordered_map<std::string, float *> _hdr_texture_array_data_cache;

	};
}

#endif // VIRTUALVISTA_TEXTUREMANAGER_H