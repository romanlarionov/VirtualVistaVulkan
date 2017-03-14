
#include "VulkanImage.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanImage::VulkanImage()
	{
	}


	VulkanImage::~VulkanImage()
	{
	}


    void VulkanImage::create(VulkanDevice *device, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkImageType type,
                             VkImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t array_layers,
                             VkImageLayout initial_layout, VkSampleCountFlagBits sample_count)
    {
		VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
		_device = device;
		this->format = format;
        this->width = extent.width;
        this->height = extent.height;
        this->depth = extent.depth;
        this->type = type;
        this->aspect_flags = aspect_flags;
        this->mip_levels = mip_levels;
        this->array_layers = array_layers;
        this->sample_count = sample_count;
        this->initial_layout = initial_layout;

		allocateMemory(VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
			           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _staging_image, _staging_memory);

		allocateMemory(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, _image_memory);
    }


	void VulkanImage::createFromImage(VulkanDevice *device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags,
                                      uint32_t mip_levels, uint32_t array_layers)
	{
		VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
		_device = device;
		this->image = image;
		this->format = format;
		this->aspect_flags = aspect_flags;
        this->mip_levels = mip_levels;
        this->array_layers = array_layers;
	}


	void VulkanImage::createDepthAttachment(VulkanDevice *device, VkExtent2D extent, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
		this->_device = device;
		this->format = format;
		this->aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
        this->type = VK_IMAGE_TYPE_2D;
		this->width = extent.width;
		this->height = extent.height;
		this->depth = 1;
        this->mip_levels = 1;
        this->array_layers = 1;

		// check to see if physical device supports the particular image format required for depth operations.
		std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
		for (VkFormat &format : candidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(device->physical_device, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			{
				this->format = format;
				break;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			{
				this->format = format;
				break;
			}
		}

		allocateMemory(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, _image_memory);

		if (hasStencilComponent())
			this->aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

		transformImageLayout(image, this->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}


	void VulkanImage::shutDown()
	{
		if (_staging_image != VK_NULL_HANDLE)
			vkDestroyImage(_device->logical_device, _staging_image, nullptr);
		if (_staging_memory != VK_NULL_HANDLE)
			vkFreeMemory(_device->logical_device, _staging_memory, nullptr);
		
		vkDestroyImage(_device->logical_device, image, nullptr);
		vkFreeMemory(_device->logical_device, _image_memory, nullptr);
	}


    void VulkanImage::updateAndTransfer(void *data, uint32_t size_in_bytes)
    {
        update(data, size_in_bytes);
        transferToDevice();
    }


    void VulkanImage::update(void *data, uint32_t size_in_bytes)
    {
		void *mapped_data;
		vkMapMemory(_device->logical_device, _staging_memory, 0, size_in_bytes, 0, &mapped_data);
		memcpy(mapped_data, data, size_in_bytes);
		vkUnmapMemory(_device->logical_device, _staging_memory);
    }


	void VulkanImage::transferToDevice()
	{
		transformImageLayout(_staging_image, format, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		transformImageLayout(image, format, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        auto command_pool_used = _device->command_pools["graphics"];

        // use transfer queue if available
        if (_device->command_pools.count("transfer") > 0)
            command_pool_used = _device->command_pools["transfer"];

		auto command_buffer = util::beginSingleUseCommand(_device->logical_device, command_pool_used);

		VkImageSubresourceLayers sub_resource = {};
		sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_resource.baseArrayLayer = 0;
		sub_resource.mipLevel = 0;
		sub_resource.layerCount = 1;

		VkImageCopy image_copy = {};
		image_copy.srcSubresource = sub_resource;
		image_copy.dstSubresource = sub_resource;
		image_copy.srcOffset = {0, 0, 0};
		image_copy.dstOffset = {0, 0, 0};
		image_copy.extent.width = this->width;
		image_copy.extent.height = this->height;
		image_copy.extent.depth = this->depth;

		vkCmdCopyImage(command_buffer, _staging_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);

        if (_device->transfer_family_index != -1)
            util::endSingleUseCommand(_device->logical_device, command_pool_used, command_buffer, _device->transfer_queue);
        else
		    util::endSingleUseCommand(_device->logical_device, command_pool_used, command_buffer, _device->graphics_queue);

		transformImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}


	bool VulkanImage::hasStencilComponent()
	{
		return (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
    void VulkanImage::allocateMemory(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkImage &image, VkDeviceMemory &memory)
	{
		VkImageCreateInfo image_create_info = {};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.flags = 0;
		image_create_info.extent.width = this->width;
		image_create_info.extent.height = this->height;
		image_create_info.extent.depth = this->depth;
		image_create_info.imageType = this->type;
		image_create_info.mipLevels = this->mip_levels;
		image_create_info.arrayLayers = this->array_layers;
		image_create_info.format = this->format; // can be changed at a later time
		image_create_info.tiling = tiling;
		image_create_info.usage = usage;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED; // VK_IMAGE_LAYOUT_UNDEFINED <- good for color/depth buffers
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT; // other options can be used to help with sparse 3d texture data

        if (_device->transfer_family_index != -1)
        {
            image_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            image_create_info.queueFamilyIndexCount = 2;
            std::array<uint32_t, 2> queue_family_indices = {
                static_cast<uint32_t>(_device->graphics_family_index),
                static_cast<uint32_t>(_device->transfer_family_index)
            };
            image_create_info.pQueueFamilyIndices = queue_family_indices.data();
        }
        else
        {
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.queueFamilyIndexCount = 1;
            uint32_t queue_index = static_cast<uint32_t>(_device->graphics_family_index);
            image_create_info.pQueueFamilyIndices = &queue_index;
        }

		VV_CHECK_SUCCESS(vkCreateImage(_device->logical_device, &image_create_info, nullptr, &image));

		// Determine requirements for memory (where it's allocated, type of memory, etc.)
		VkMemoryRequirements memory_requirements = {};
		vkGetImageMemoryRequirements(_device->logical_device, image, &memory_requirements);
		auto memory_type = _device->findMemoryTypeIndex(memory_requirements.memoryTypeBits, memory_properties);

		// Allocate and bind buffer memory.
		VkMemoryAllocateInfo memory_allocate_info = {};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = memory_type;

		VV_CHECK_SUCCESS(vkAllocateMemory(_device->logical_device, &memory_allocate_info, nullptr, &memory));
		vkBindImageMemory(_device->logical_device, image, memory, 0);
	}


	void VulkanImage::transformImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{
        auto command_pool_used = _device->command_pools["graphics"];

        // use transfer queue if available
        if (_device->command_pools.count("transfer") > 0)
            command_pool_used = _device->command_pools["transfer"];

		auto command_buffer = util::beginSingleUseCommand(_device->logical_device, command_pool_used);

		// using this ensures that writing is completed before reading.
		VkImageMemoryBarrier memory_barrier = {};
		memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memory_barrier.oldLayout = old_layout;
		memory_barrier.newLayout = new_layout;
		memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.image = image;

		// todo: make all of below more general
		memory_barrier.subresourceRange.aspectMask = this->aspect_flags;
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
		} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			memory_barrier.srcAccessMask = 0;
			memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		} else {
			VV_ASSERT(false, "Unsupported image layout transformation.");
		}

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &memory_barrier);

        if (_device->transfer_family_index != -1)
            util::endSingleUseCommand(_device->logical_device, command_pool_used, command_buffer, _device->transfer_queue);
        else
		    util::endSingleUseCommand(_device->logical_device, command_pool_used, command_buffer, _device->graphics_queue);
	}
}