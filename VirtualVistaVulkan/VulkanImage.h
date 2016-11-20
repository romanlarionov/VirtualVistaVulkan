
#ifndef VIRTUALVISTA_VULKANIMAGE_H
#define VIRTUALVISTA_VULKANIMAGE_H

#include <vulkan\vulkan.h>
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
		std::string path;

		VulkanImage();
		~VulkanImage();

		/*
		 * Creates an image from existing image. Mainly for swap chain image support.
		 */
		void create(VkImage image, VulkanDevice *device, VkFormat format);

		/*
		 * Loads, allocates, and stores a requested texture using the header-only STB image library.
		 */
		void create(std::string path, VulkanDevice *device, VkFormat format);

		/*
		 *
		 */
		void shutDown();
	
		/*
		 * todo: should be able to load different types of textures such as,
		 * 3d textures, generated textures, and deferred staging buffers.
		 */
		/*void create(VulkanDevice *device)
		{

		}*/

		/*
		 * Copies a buffer allocated on CPU memory to one allocated on GPU memory.
		 * todo: this issues a lot of commands that are sent to be executed linearly. These should be done asynchronously for best performance.
		 */
		void transferToDevice();

	private:
		VulkanDevice *device_;
		VkImage staging_image_ = VK_NULL_HANDLE;
		VkDeviceMemory staging_memory_ = VK_NULL_HANDLE;
		VkDeviceMemory image_memory_;
		VkDeviceSize size_; // width * height * num_elements
		int width_;
		int height_;
		int depth_;

		/*
		 * Creates the Vulkan abstraction for a data buffer with the given specifications.
		 * todo: maybe think about putting this sort of thing in its own "memory management" system
		 */
		void allocateMemory(VkImageType image_type, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkImage &image, VkDeviceMemory &memory);
	
		/*
		 * Move the linearly stored staging image into an optimal texture storage layout.
		 * https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkImageLayout
		 */
		void transformImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
	};
}

#endif // VIRTUALVISTA_VULKANIMAGE_H