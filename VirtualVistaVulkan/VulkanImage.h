
#ifndef VIRTUALVISTA_VULKANIMAGE_H
#define VIRTUALVISTA_VULKANIMAGE_H

#include <vulkan\vulkan.h>
#include <vector>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Utils.h"
#include "VulkanDevice.h"

namespace vv
{
	class VulkanImage
	{
	public:
		VkImage image;

		VulkanImage() {}

		~VulkanImage()
		{
			if (staging_image_)
				vkDestroyImage(device_->logical_device, staging_image_, nullptr);
			if (staging_memory_)
				vkFreeMemory(device_->logical_device, staging_memory_, nullptr);
			
			vkDestroyImage(device_->logical_device, image, nullptr);
			vkFreeMemory(device_->logical_device, image_memory_, nullptr);
		}

		/*
		 * Loads, allocates, and stores a requested texture using the header-only STB image library.
		 */
		void create(VulkanDevice *device, VkFormat format, std::string path = "")
		{
			VV_ASSERT(device, "VulkanDevice not present");
			device_ = device;

			int channels;

			// todo: have conversion between VkFormat and STB format:
			int stb_format = (format == VK_FORMAT_R8G8B8A8_UNORM) ? STBI_rgb_alpha : 0;

			// loads the image into a 1d array w/ 4 byte channel elements.
			stbi_uc *texels = stbi_load(path.c_str(), &width_, &height_, &channels, stb_format); // todo: maybe make picking channel type up to caller
			size_ = width_ * height_ * 4; // todo: change depending on passed in channel size

			VV_ASSERT(texels, "STB failed to load image: " + path + " ");

			allocateMemory(VK_IMAGE_TYPE_2D, format, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_image_, staging_memory_);

			allocateMemory(VK_IMAGE_TYPE_2D, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory_);

			// Update the staging image with loaded texture data
			void *mapped_data;
			vkMapMemory(device_->logical_device, staging_memory_, 0, size_, 0, &mapped_data);
			memcpy(mapped_data, texels, size_);
			vkUnmapMemory(device_->logical_device, staging_memory_);

			// free loaded raw data
			stbi_image_free(texels);
		}

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
		void transferToDevice()
		{
			transformImageLayout(staging_image_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			transformImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			auto command_buffer = vv::beginSingleUseCommand(device_->logical_device, device_->command_pools["transfer"]);

			VkImageSubresourceLayers sub_resource = {};
			sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			sub_resource.baseArrayLayer = 0;
			sub_resource.mipLevel = 0;
			sub_resource.layerCount = 1;

			VkImageCopy region = {};
			region.srcSubresource = sub_resource;
			region.dstSubresource = sub_resource;
			region.srcOffset = {0, 0, 0};
			region.dstOffset = {0, 0, 0};
			region.extent.width = width_;
			region.extent.height = height_;
			region.extent.depth = 1;

			vkCmdCopyImage(command_buffer, staging_image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			vv::endSingleUseCommand(command_buffer, device_->transfer_queue);

			transformImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

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
		void allocateMemory(VkImageType image_type, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits memory_properties, VkImage image, VkDeviceMemory memory)
		{
			VkImageCreateInfo image_create_info = {};
			image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_create_info.imageType = image_type;
			image_create_info.extent.width = width_;
			image_create_info.extent.height = height_;
			image_create_info.extent.depth = depth_;
			image_create_info.mipLevels = 1; // todo: change to be more general
			image_create_info.arrayLayers = 1;
			image_create_info.format = format; // can be changed at a later time
			image_create_info.tiling = tiling;
			image_create_info.usage = usage;
			image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED; // VK_IMAGE_LAYOUT_UNDEFINED <- good for color/depth buffers
			image_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
			image_create_info.queueFamilyIndexCount = 2;
			std::array<uint32_t, 2> queue_family_indices = { device_->graphics_family_index, device_->transfer_family_index };
			image_create_info.pQueueFamilyIndices = queue_family_indices.data();
			image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_create_info.flags = 0;

			VV_CHECK_SUCCESS(vkCreateImage(device_->logical_device, &image_create_info, nullptr, &image));

			// Determine requirements for memory (where it's allocated, type of memory, etc.)
			VkMemoryRequirements memory_requirements = {};
			vkGetImageMemoryRequirements(device_->logical_device, image, &memory_requirements);
			auto memory_type = device_->findMemoryTypeIndex(memory_requirements.memoryTypeBits, memory_properties);

			// Allocate and bind buffer memory.
			VkMemoryAllocateInfo memory_allocate_info = {};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.allocationSize = memory_requirements.size;
			memory_allocate_info.memoryTypeIndex = memory_type;

			VV_CHECK_SUCCESS(vkAllocateMemory(device_->logical_device, &memory_allocate_info, nullptr, &image_memory_));
			vkBindImageMemory(device_->logical_device, image, memory, 0);
		}

		/*
		 * Move the linearly stored staging image into an optimal texture storage layout.
		 * https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkImageLayout
		 */
		void transformImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
		{
			auto command_buffer = vv::beginSingleUseCommand(device_->logical_device, device_->command_pools["graphics"]);

			// using this ensures that writing is completed before reading.
			VkImageMemoryBarrier memory_barrier = {};
			memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memory_barrier.oldLayout = old_layout;
			memory_barrier.newLayout = new_layout;
			memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memory_barrier.image = image;
			// todo: make all of below more general
			memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // what parts of the image are included?
			memory_barrier.subresourceRange.baseMipLevel = 0;
			memory_barrier.subresourceRange.levelCount = 1;
			memory_barrier.subresourceRange.baseArrayLayer = 0;
			memory_barrier.subresourceRange.layerCount = 1;

			// todo: maybe put in its own function:
			if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
				memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			} else if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			} else {
				VV_ASSERT(false, "Unsupported image layout transformation.");
			}

			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				0, 0, nullptr, 0, nullptr, 1, &memory_barrier);

			vv::endSingleUseCommand(command_buffer, device_->graphics_queue);
		}
	};
}

#endif // VIRTUALVISTA_VULKANIMAGE_H