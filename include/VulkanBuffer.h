
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
        VkDeviceSize size;

		VulkanBuffer();
		~VulkanBuffer();

		/*
		 * Creates two VkBuffers. One as a transfer buffer located on CPU memory and one as a storage buffer on GPU memory.
		 * Use along with update() and transferToDevice().
		 */
		void create(VulkanDevice *device, VkBufferUsageFlags usage_flags, VkDeviceSize size);

        /*
         *
         */
		void shutDown();

		/*
		 * Helper function to perform an update and transfer in a single step.
		 */
		void updateAndTransfer(void *data);
		
		/*
		 * Updates the staging buffer with new raw data.
		 */
		void update(void *data);
		
		/*
		 * Copies a buffer allocated on CPU memory to one allocated on GPU memory.
		 */
		void transferToDevice();
		
	private:
		VulkanDevice *_device;
		VkBuffer _staging_buffer = VK_NULL_HANDLE;
		VkDeviceMemory _staging_memory = VK_NULL_HANDLE;
		VkDeviceMemory _buffer_memory;
		VkBufferUsageFlags _usage_flags;

		/*
		 * Creates the Vulkan abstraction for a data buffer with the given specifications.
		 */
		void allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory);
	};
}

#endif // VIRTUALVISTA_VULKANBUFFER_H