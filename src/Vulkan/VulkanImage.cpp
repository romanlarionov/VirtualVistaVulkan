
#include "VulkanImage.h"
#include "Utils.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanImage::VulkanImage()
	{
	}


	VulkanImage::~VulkanImage()
	{
	}


    void VulkanImage::create(VulkanDevice *device, VkExtent3D extent, VkFormat format, VkImageType type, VkImageCreateFlags flags,
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

        allocateMemory(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            flags, initial_layout, sample_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, _image_memory);
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

		allocateMemory(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED,
                       VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, _image_memory);

		if (hasStencilComponent())
			this->aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

        VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = aspect_flags;
		subresource_range.levelCount = 1;
		subresource_range.layerCount = 1;
		transformImageLayout(image, subresource_range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}


	void VulkanImage::shutDown()
	{
		vkDestroyImage(_device->logical_device, image, nullptr);
		vkFreeMemory(_device->logical_device, _image_memory, nullptr);
	}

    
    void VulkanImage::updateAndTransfer(void *data, VkDeviceSize size_in_bytes)
    {
        auto command_pool_used = _device->command_pools["graphics"];
        auto command_buffer = util::beginSingleUseCommand(_device->logical_device, command_pool_used);

        VkImageSubresourceRange subresource_range = {};
        subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.baseMipLevel = 0;
        subresource_range.levelCount = this->mip_levels;
        subresource_range.layerCount = this->array_layers;

        transformImageLayout(command_buffer, image, subresource_range, initial_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // move host data to transfer buffer
        allocateTransferMemory(size_in_bytes);
        void *mapped_data;
        vkMapMemory(_device->logical_device, _staging_memory, 0, size_in_bytes, 0, &mapped_data);
        memcpy(mapped_data, data, size_in_bytes);
        vkUnmapMemory(_device->logical_device, _staging_memory);
        mapped_data = nullptr;

        const auto &format_info = _format_info_table.at(format);
		const uint32_t block_size = format_info.block_size;
		const uint32_t block_width = format_info.block_extent.width;
		const uint32_t block_height = format_info.block_extent.height;
		const uint32_t block_depth = format_info.block_extent.depth;

		// Copy mip levels from staging buffer
		std::vector<VkBufferImageCopy> buffer_copy_regions;
		uint32_t offset = 0;

		for (uint32_t layer = 0; layer < this->array_layers; layer++)
		{
			for (uint32_t level = 0; level < this->mip_levels; level++)
			{
				uint32_t image_width = (this->width >> level);
				uint32_t image_height = (this->height >> level);
				uint32_t block_count_x = (image_width + (block_width - 1)) / block_width;
				uint32_t block_count_y = (image_height + (block_height - 1)) / block_height;
				uint32_t block_count_z = (depth + (block_depth - 1)) / block_depth;

				VkBufferImageCopy buffer_copy_region = {};
				buffer_copy_region.imageSubresource.aspectMask = this->aspect_flags;
				buffer_copy_region.imageSubresource.mipLevel = level;
				buffer_copy_region.imageSubresource.baseArrayLayer = layer;
				buffer_copy_region.imageSubresource.layerCount = 1;
				buffer_copy_region.imageExtent.width = image_width;
				buffer_copy_region.imageExtent.height = image_height;
				buffer_copy_region.imageExtent.depth = depth;
				buffer_copy_region.bufferOffset = offset;

				buffer_copy_regions.push_back(buffer_copy_region);
				offset += block_count_x * block_count_y * block_count_z * block_size;
			}
		}

		vkCmdCopyBufferToImage(command_buffer, _staging_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(buffer_copy_regions.size()), buffer_copy_regions.data());

        transformImageLayout(command_buffer, image, subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        util::endSingleUseCommand(_device->logical_device, command_pool_used, command_buffer, _device->graphics_queue);

        vkDestroyBuffer(_device->logical_device, _staging_buffer, nullptr);
        vkFreeMemory(_device->logical_device, _staging_memory, nullptr);
    }


	bool VulkanImage::hasStencilComponent()
	{
		return (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
    void VulkanImage::allocateTransferMemory(VkDeviceSize size_in_bytes)
    {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.flags = 0;
        buffer_create_info.size = size_in_bytes;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VV_CHECK_SUCCESS(vkCreateBuffer(_device->logical_device, &buffer_create_info, nullptr, &_staging_buffer));

		VkMemoryRequirements memory_requirements = {};
		vkGetBufferMemoryRequirements(_device->logical_device, _staging_buffer, &memory_requirements);

        VkMemoryAllocateInfo memory_alloc_info = {};
        memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_alloc_info.allocationSize = memory_requirements.size;
		memory_alloc_info.memoryTypeIndex = _device->findMemoryTypeIndex(memory_requirements.memoryTypeBits,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VV_CHECK_SUCCESS(vkAllocateMemory(_device->logical_device, &memory_alloc_info, nullptr, &_staging_memory));
		VV_CHECK_SUCCESS(vkBindBufferMemory(_device->logical_device, _staging_buffer, _staging_memory, 0));
    }


    void VulkanImage::allocateMemory(VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageLayout initial_layout,
                                     VkSampleCountFlagBits sample_count, VkMemoryPropertyFlags memory_properties, VkImage &image,
                                     VkDeviceMemory &memory)
	{
		VkImageCreateInfo image_create_info = {};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.flags = flags;
		image_create_info.extent.width = this->width;
		image_create_info.extent.height = this->height;
		image_create_info.extent.depth = this->depth;
		image_create_info.imageType = this->type;
		image_create_info.mipLevels = this->mip_levels;
		image_create_info.arrayLayers = this->array_layers;
		image_create_info.format = this->format; // can be changed at a later time
		image_create_info.tiling = tiling;
		image_create_info.usage = usage;
        image_create_info.initialLayout = initial_layout;
        image_create_info.samples = sample_count;

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
		VV_CHECK_SUCCESS(vkBindImageMemory(_device->logical_device, image, memory, 0));
	}


    void VulkanImage::transformImageLayout(VkCommandBuffer command_buffer, VkImage image, VkImageSubresourceRange subresource_range, VkImageLayout old_layout, VkImageLayout new_layout)
    {
        VkImageMemoryBarrier memory_barrier = determineAccessMasks(image, subresource_range, old_layout, new_layout);
        VkPipelineStageFlags old_stage = util::determinePipelineStageFlag(memory_barrier.srcAccessMask);
        VkPipelineStageFlags new_stage = util::determinePipelineStageFlag(memory_barrier.dstAccessMask);
		vkCmdPipelineBarrier(command_buffer, old_stage, new_stage, 0, 0, nullptr, 0, nullptr, 1, &memory_barrier);
    }


	void VulkanImage::transformImageLayout(VkImage image, VkImageSubresourceRange subresource_range, VkImageLayout old_layout, VkImageLayout new_layout)
	{
		// using this ensures that writing is completed before reading.
        VkImageMemoryBarrier memory_barrier = determineAccessMasks(image, subresource_range, old_layout, new_layout);
        VkPipelineStageFlags old_stage = util::determinePipelineStageFlag(memory_barrier.srcAccessMask);
        VkPipelineStageFlags new_stage = util::determinePipelineStageFlag(memory_barrier.dstAccessMask);

        bool use_transfer = false;
        auto command_pool_used = _device->command_pools["graphics"];

        // use transfer queue if it's available on this device and the old/new pipeline stages are compatible with it
        if ((_pipeline_stage_queue_support_lut[old_stage] == VK_QUEUE_TRANSFER_BIT) &&
            (_pipeline_stage_queue_support_lut[new_stage] == VK_QUEUE_TRANSFER_BIT) &&
            _device->command_pools.count("transfer") > 0)
        {
            command_pool_used = _device->command_pools["transfer"];
            use_transfer = true;
        }

		auto command_buffer = util::beginSingleUseCommand(_device->logical_device, command_pool_used);
		vkCmdPipelineBarrier(command_buffer, old_stage, new_stage, 0, 0, nullptr, 0, nullptr, 1, &memory_barrier);

        if (use_transfer)
            util::endSingleUseCommand(_device->logical_device, command_pool_used, command_buffer, _device->transfer_queue);
        else
		    util::endSingleUseCommand(_device->logical_device, command_pool_used, command_buffer, _device->graphics_queue);
	}


    VkImageMemoryBarrier VulkanImage::determineAccessMasks(VkImage image, VkImageSubresourceRange subresource_range, VkImageLayout old_layout, VkImageLayout new_layout)
    {
		VkImageMemoryBarrier memory_barrier = {};
        memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memory_barrier.oldLayout = old_layout;
		memory_barrier.newLayout = new_layout;
        //memory_barrier.srcAccessMask = this->aspect_flags;
        //memory_barrier.dstAccessMask = this->aspect_flags;
		memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.image = image;
        memory_barrier.subresourceRange = subresource_range;

        // https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanTools.cpp#L94
        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (old_layout)
        {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                memory_barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source 
                // Make sure any reads from the image have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
		switch (new_layout)
		{
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from and writes to the image have been finished
                memory_barrier.srcAccessMask = memory_barrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
                memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                memory_barrier.dstAccessMask = memory_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (memory_barrier.srcAccessMask == 0)
                	memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

                memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
        }

        return memory_barrier;
    }
}