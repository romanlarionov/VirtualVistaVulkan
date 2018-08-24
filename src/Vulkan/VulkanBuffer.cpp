
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
        m_device = device;
        m_usage_flags = usage_flags;
        this->size = size;

        // Create temporary transfer buffer on CPU 
        allocateMemory(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_staging_buffer, m_staging_memory);

        // Create storage buffer for GPU
        allocateMemory(size, usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, m_buffer_memory);	
    }


    void VulkanBuffer::shutDown()
    {
        if (m_staging_buffer)
        	vkDestroyBuffer(m_device->logical_device, m_staging_buffer, nullptr);
        if (m_staging_memory)
        	vkFreeMemory(m_device->logical_device, m_staging_memory, nullptr);

        vkDestroyBuffer(m_device->logical_device, buffer, nullptr);
        vkFreeMemory(m_device->logical_device, m_buffer_memory, nullptr);
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
        vkMapMemory(m_device->logical_device, m_staging_memory, 0, size, 0, &mapped_data);
        memcpy(mapped_data, data, size);
        vkUnmapMemory(m_device->logical_device, m_staging_memory);
    }


    void VulkanBuffer::transferToDevice()
    {
        VV_ASSERT(m_staging_buffer && buffer, "Buffers not allocated correctly. Perhaps create() wasn't called.");

        bool use_transfer = false;
        auto command_pool_used = m_device->command_pools["graphics"];

        // use transfer queue if available
        if (m_device->command_pools.count("transfer") > 0)
        {
            command_pool_used = m_device->command_pools["transfer"];
            use_transfer = true;
        }

        auto command_buffer = util::beginSingleUseCommand(m_device->logical_device, command_pool_used);

        VkBufferCopy buffer_copy = {};
        buffer_copy.size = size;
        vkCmdCopyBuffer(command_buffer, m_staging_buffer, buffer, 1, &buffer_copy);

        if (use_transfer)
            util::endSingleUseCommand(m_device->logical_device, command_pool_used, command_buffer, m_device->transfer_queue);
        else
            util::endSingleUseCommand(m_device->logical_device, command_pool_used, command_buffer, m_device->graphics_queue);
    }


    ///////////////////////////////////////////////////////////////////////////////////////////// Private
    void VulkanBuffer::allocateMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory)
    {
        // Create the Vulkan abstraction for a vertex buffer.
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.flags = 0; // can be used to specify this stores sparse data
        buffer_create_info.size = size;
        buffer_create_info.usage = usage; // use this as a vertex/index buffer

        // use transfer queue if available
        if (m_device->transfer_family_index != -1)
        {
            buffer_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            buffer_create_info.queueFamilyIndexCount = 2;
            std::array<uint32_t, 2> queue_family_indices = {
                static_cast<uint32_t>(m_device->graphics_family_index),
                static_cast<uint32_t>(m_device->transfer_family_index)
            };
            buffer_create_info.pQueueFamilyIndices = queue_family_indices.data();
        }
        else
        {
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_create_info.queueFamilyIndexCount = 1;
            uint32_t queue_index = static_cast<uint32_t>(m_device->graphics_family_index);
            buffer_create_info.pQueueFamilyIndices = &queue_index;
        }

        VV_CHECK_SUCCESS(vkCreateBuffer(m_device->logical_device, &buffer_create_info, nullptr, &buffer));

        // Determine requirements for memory (where it's allocated, type of memory, etc.)
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(m_device->logical_device, buffer, &memory_requirements);
        auto memory_type = m_device->findMemoryTypeIndex(memory_requirements.memoryTypeBits, memory_properties);

        // Allocate and bind buffer memory.
        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_type;

        VV_CHECK_SUCCESS(vkAllocateMemory(m_device->logical_device, &memory_allocate_info, nullptr, &buffer_memory));
        vkBindBufferMemory(m_device->logical_device, buffer, buffer_memory, 0);
    }
}