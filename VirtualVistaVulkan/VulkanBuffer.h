
#ifndef VIRTUALVISTA_VULKANBUFFER_H
#define VIRTUALVISTA_VULKANBUFFER_H

#include <vulkan\vulkan.h>
#include <vector>
#include <array>

#include "Utils.h"
#include "VulkanDevice.h"

namespace vv
{
	class VulkanBuffer
	{
	public:
		VkBuffer buffer;

		VulkanBuffer() {}

		~VulkanBuffer()
		{
			if (staging_buffer_)
				vkDestroyBuffer(device_->logical_device, staging_buffer_, nullptr);
			if (staging_memory_)
				vkFreeMemory(device_->logical_device, staging_memory_, nullptr);

			vkDestroyBuffer(device_->logical_device, buffer, nullptr);
			vkFreeMemory(device_->logical_device, buffer_memory_, nullptr);
		}

		/*
		 * Creates two VkBuffers. One as a transfer buffer located on CPU memory and one as a storage buffer on GPU memory.
		 * Use along with update() and transferToDevice().
		 */
		void create(VulkanDevice *device, VkBufferUsageFlags usage_flags, VkDeviceSize size)
		{
			VV_ASSERT(device, "VulkanDevice not present");
			device_ = device;
			usage_flags_ = usage_flags;
			size_ = size;

			// Create temporary transfer buffer on CPU 
			allocateMemory(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer_, staging_memory_);

			// Create storage buffer for GPU
			allocateMemory(size, usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory_);
		}

		/*
		 * Updates the staging buffer with new raw data.
		 */
		void update(void *data)
		{
			// Move raw data to staging Vulkan buffer.
			void *mapped_data;
			vkMapMemory(device_->logical_device, staging_memory_, 0, size_, 0, &mapped_data);
			memcpy(mapped_data, data, size_);
			vkUnmapMemory(device_->logical_device, staging_memory_);
		}

		/*
		 * Copies a buffer allocated on CPU memory to one allocated on GPU memory.
		 */
		void transferToDevice()
		{
			VV_ASSERT(staging_buffer_ && buffer, "Buffers not allocated correctly. Perhaps create() wasn't called.");

			// Create a special command buffer to perform the transfer operation
			VkCommandBufferAllocateInfo allocate_info = {};
			allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocate_info.commandPool = device_->command_pools["transfer"];
			allocate_info.commandBufferCount = 1;

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(device_->logical_device, &allocate_info, &command_buffer);

			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // this buffer will be used once then discarded

			vkBeginCommandBuffer(command_buffer, &begin_info);

			VkBufferCopy buffer_copy = {};
			buffer_copy.size = size_;
			vkCmdCopyBuffer(command_buffer, staging_buffer_, buffer, 1, &buffer_copy);
			vkEndCommandBuffer(command_buffer);

			VkSubmitInfo submit_info = {};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer;

			// I could use a fence here to possibly wait for a series of copies to finish instead of this single one
			vkQueueSubmit(device_->transfer_queue, 1, &submit_info, nullptr);
			vkQueueWaitIdle(device_->transfer_queue);

			// free the buffer memory
			vkFreeCommandBuffers(device_->logical_device, device_->command_pools["transfer"], 1, &command_buffer);
		}

	private:
		VulkanDevice *device_;
		VkBuffer staging_buffer_ = VK_NULL_HANDLE;
		VkDeviceMemory staging_memory_ = VK_NULL_HANDLE;
		VkDeviceMemory buffer_memory_;
		VkBufferUsageFlags usage_flags_;
		VkDeviceSize size_;

		/*
		 * Creates the Vulkan abstraction for a data buffer with the given specifications.
		 */
		void allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory)
		{
			// Create the Vulkan abstraction for a vertex buffer.
			VkBufferCreateInfo buffer_create_info = {};
			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.size = size;
			buffer_create_info.usage = usage; // use this as a vertex/index buffer
			buffer_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT; // buffers can be used in queues. choice between exclusive or shared
			buffer_create_info.queueFamilyIndexCount = 2;
			std::array<uint32_t, 2> queue_family_indices = { device_->graphics_family_index, device_->transfer_family_index };
			buffer_create_info.pQueueFamilyIndices = queue_family_indices.data();
			buffer_create_info.flags = 0; // can be used to specify this stores sparse data

			VV_CHECK_SUCCESS(vkCreateBuffer(device_->logical_device, &buffer_create_info, nullptr, &buffer));

			// Determine requirements for memory (where it's allocated, type of memory, etc.)
			VkMemoryRequirements memory_requirements = {};
			vkGetBufferMemoryRequirements(device_->logical_device, buffer, &memory_requirements);
			auto memory_type = findMemoryType(memory_requirements.memoryTypeBits, memory_properties);

			// Allocate and bind buffer memory.
			VkMemoryAllocateInfo memory_allocate_info = {};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.allocationSize = memory_requirements.size;
			memory_allocate_info.memoryTypeIndex = memory_type;

			// todo: single allocations are costly. figure out how to batch data together. (might just do that logic outside of this class)
			VV_CHECK_SUCCESS(vkAllocateMemory(device_->logical_device, &memory_allocate_info, nullptr, &buffer_memory));
			vkBindBufferMemory(device_->logical_device, buffer, buffer_memory, 0);
		}

		/*
		 * Finds the index for the appropriate supported memory type for the given physical device.
		 */
		uint32_t findMemoryType(uint32_t filter_type, VkMemoryPropertyFlags memory_property_flags)
		{
			// todo: check for validity and robustness.
			auto memory_properties = device_->physical_device_memory_properties;
			for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
			{
				// Check all memory heaps to find memory type that fulfills out needs and has the correct properties.
				if ((filter_type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags)
					return i;
			}

			VV_ASSERT(false, "Couldn't find appropriate memory type");
		}
	};
}

#endif // VIRTUALVISTA_VULKANBUFFER_H