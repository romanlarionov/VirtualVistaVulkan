
#ifndef VIRTUALVISTA_VULKANIMAGE_H
#define VIRTUALVISTA_VULKANIMAGE_H

#include <vector>
#include <array>
#include <string>

#include "Utils.h"
#include "VulkanDevice.h"

namespace vv
{
	class VulkanImage
	{
	public:
		VkImage image;
		VkFormat format;
		VkImageAspectFlags aspect_flags;
        VkImageType type;
        uint32_t mip_levels;
        uint32_t array_layers;
        VkSampleCountFlagBits sample_count;
        VkImageLayout initial_layout;
        int width;
		int height;
		int depth;

		VulkanImage();
		~VulkanImage();

        /*
         * 
         */
        void create(VulkanDevice *device, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkImageType type,
                    VkImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t array_layers,
                    VkImageLayout initial_layout, VkSampleCountFlagBits sample_count);

        /*
		 * Creates an image from existing image. Mainly for swap chain image support.
		 */
	    void createFromImage(VulkanDevice *device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags,
                             uint32_t mip_levels, uint32_t array_layers);

        /*
         * Creates a special image intended to be used as a depth/stencil attachment to a framebuffer.
         */
        void createDepthAttachment(VulkanDevice *device, VkExtent2D extent, VkImageTiling tiling, VkFormatFeatureFlags features);

		/*
		 * Removes allocated device memory.
		 */
		void shutDown();
	
        /*
         * Performs update and transfer operation in single step.
         */
        void updateAndTransfer(void *data, uint32_t size_in_bytes);

        /*
         * Updates the allocated device memory with data provided.
         */
        void update(void *data, uint32_t size_in_bytes);

		/*
		 * Copies a buffer allocated on CPU memory to one allocated on GPU memory.
		 *
         * todo: This issues a lot of commands that are sent to be executed linearly.
         *       These should be done asynchronously for best performance.
		 */
		void transferToDevice();

		/*
		 * Returns whether this image format supports stencil operations.
		 */
		bool hasStencilComponent();

	private:
		VulkanDevice *_device;
		VkImage _staging_image			= VK_NULL_HANDLE;
		VkDeviceMemory _staging_memory	= VK_NULL_HANDLE;
		VkDeviceMemory _image_memory	= VK_NULL_HANDLE;
		
		/*
		 * Creates the Vulkan abstraction for a data buffer with the given specifications.
         *
		 * todo: maybe think about putting this sort of thing in its own "memory management" system
		 */
        void allocateMemory(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties,
                            VkImage &image, VkDeviceMemory &memory);
	
		/*
		 * Move the linearly stored staging image into an optimal texture storage layout.
		 * https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkImageLayout
		 */
		void transformImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
	};
}

#endif // VIRTUALVISTA_VULKANIMAGE_H