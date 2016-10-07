
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
			vkDestroyBuffer(device_->logical_device, buffer, nullptr); // todo: ensure that the temp buffers are destroyed too.
		}

		void create(VulkanDevice *device, VkCommandPool command_pool, const std::vector<Vertex> vertices)
		{
			VV_ASSERT(device, "VulkanDevice not present");
			VV_ASSERT(command_pool, "VkCommandPool not present");
			device_ = device;
			command_pool_ = command_pool;

			VkDeviceSize size = sizeof(vertices[0]) * vertices.size();
			VkBuffer staging_buffer;
			VkDeviceMemory staging_memory;
			VkMemoryPropertyFlags prop = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			// Create temporary transfer buffer on CPU 
			createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, prop, staging_buffer, staging_memory);

			// Move data from host to device.
			void *data;
			vkMapMemory(device_->logical_device, staging_memory, 0, size, 0, &data);
			memcpy(data, vertices.data(), (size_t)size);
			vkUnmapMemory(device_->logical_device, staging_memory);

			// Create storage buffer for GPU
			createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory_);

			copyBuffer(staging_buffer, buffer, size);
		}

		void create(VulkanDevice *device, VkCommandPool command_pool, const std::vector<uint32_t> indices)
		{
			VV_ASSERT(device, "VulkanDevice not present");
			VV_ASSERT(command_pool, "VkCommandPool not present");
			device_ = device;
			command_pool_ = command_pool;

			VkDeviceSize size = sizeof(indices[0]) * indices.size();
			VkBuffer staging_buffer;
			VkDeviceMemory staging_memory;
			VkMemoryPropertyFlags prop = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			// Create temporary transfer buffer on CPU 
			createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, prop, staging_buffer, staging_memory);

			// Move data from host to device.
			void *data;
			vkMapMemory(device_->logical_device, staging_memory, 0, size, 0, &data);
			memcpy(data, indices.data(), (size_t)size);
			vkUnmapMemory(device_->logical_device, staging_memory);

			// Create storage buffer for GPU
			createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory_);

			copyBuffer(staging_buffer, buffer, size);
		}
			
	private:
		VulkanDevice *device_;
		VkCommandPool command_pool_;
		VkDeviceMemory buffer_memory_;

		/*
		 * Creates the Vulkan abstraction for a data buffer with the given specifications.
		 */
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory)
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
		 * Copies a buffer allocated on CPU memory to one allocated on GPU memory.
		 */
		void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
		{
			// Create a special command buffer to perform the transfer operation
			VkCommandBufferAllocateInfo allocate_info = {};
			allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocate_info.commandPool = command_pool_;
			allocate_info.commandBufferCount = 1;

			VkCommandBuffer buffer;
			vkAllocateCommandBuffers(device_->logical_device, &allocate_info, &buffer);

			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // this buffer will be used once then discarded

			vkBeginCommandBuffer(buffer, &begin_info);

			VkBufferCopy buffer_copy = {};
			buffer_copy.size = size;
			vkCmdCopyBuffer(buffer, src, dst, 1, &buffer_copy);
			vkEndCommandBuffer(buffer);

			VkSubmitInfo submit_info = {};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &buffer;

			// I could use a fence here to possibly wait for a series of copies to finish instead of this single one
			vkQueueSubmit(device_->transfer_queue, 1, &submit_info, nullptr);
			vkQueueWaitIdle(device_->transfer_queue);

			// free the buffer memory
			vkFreeCommandBuffers(device_->logical_device, command_pool_, 1, &buffer);
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