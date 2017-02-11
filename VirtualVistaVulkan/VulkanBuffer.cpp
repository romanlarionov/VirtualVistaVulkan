
#include "VulkanBuffer.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanBuffer::VulkanBuffer()
	{
	}


	VulkanBuffer::~VulkanBuffer()
	{
	}


	void VulkanBuffer::create(VulkanDevice *device, VkBufferUsageFlags usage_flags, VkDeviceSize size)
	{
		VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
		device_ = device;
		usage_flags_ = usage_flags;
        size_ = size;

		// Create temporary transfer buffer on CPU 
		allocateMemory(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer_, staging_memory_);

		// Create storage buffer for GPU
		allocateMemory(size, usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory_);	
	}


	void VulkanBuffer::shutDown()
	{
		if (staging_buffer_)
			vkDestroyBuffer(device_->logical_device, staging_buffer_, nullptr);
		if (staging_memory_)
			vkFreeMemory(device_->logical_device, staging_memory_, nullptr);

		vkDestroyBuffer(device_->logical_device, buffer, nullptr);
		vkFreeMemory(device_->logical_device, buffer_memory_, nullptr);
	}


	void VulkanBuffer::updateAndTransfer(void *data)
	{
		update(data);
		transferToDevice();
	}

	
	void VulkanBuffer::update(void *data)
	{
		// Move raw data to staging Vulkan buffer.
		void *mapped_data;
		vkMapMemory(device_->logical_device, staging_memory_, 0, size_, 0, &mapped_data);
		memcpy(mapped_data, data, size_);
		vkUnmapMemory(device_->logical_device, staging_memory_);
	}


	void VulkanBuffer::transferToDevice()
	{
		VV_ASSERT(staging_buffer_ && buffer, "Buffers not allocated correctly. Perhaps create() wasn't called.");

		auto command_pool_used = device_->command_pools["transfer"];
		auto command_buffer = util::beginSingleUseCommand(device_->logical_device, command_pool_used);

		VkBufferCopy buffer_copy = {};
		buffer_copy.size = size_;
		vkCmdCopyBuffer(command_buffer, staging_buffer_, buffer, 1, &buffer_copy);

		util::endSingleUseCommand(device_->logical_device, command_pool_used, command_buffer, device_->transfer_queue);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
	void VulkanBuffer::allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory)
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
		auto memory_type = device_->findMemoryTypeIndex(memory_requirements.memoryTypeBits, memory_properties);

		// Allocate and bind buffer memory.
		VkMemoryAllocateInfo memory_allocate_info = {};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = memory_type;

		// todo: single allocations are costly. figure out how to batch data together. (might just do that logic outside of this class)
		VV_CHECK_SUCCESS(vkAllocateMemory(device_->logical_device, &memory_allocate_info, nullptr, &buffer_memory));
		vkBindBufferMemory(device_->logical_device, buffer, buffer_memory, 0);
	}
}