
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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


	void VulkanImage::createFromImage(VkImage image, VulkanDevice *device, VkFormat format)
	{
		VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
		device_ = device;
		this->format = format;
		this->aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
		this->image_view_type = VK_IMAGE_VIEW_TYPE_2D;
		this->path = "";
		this->image = image;
	}


	void VulkanImage::createColorAttachment(std::string path, VulkanDevice *device, VkFormat format)
	{
		VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
		device_ = device;
		this->format = format;
		this->aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
		this->image_view_type = VK_IMAGE_VIEW_TYPE_2D;
		this->path = path;

		int channels;

		// todo: have conversion between VkFormat and STB format:
		int stb_format = (format == VK_FORMAT_R8G8B8A8_UNORM) ? STBI_rgb_alpha : 0;

		// loads the image into a 1d array w/ 4 byte channel elements.
		unsigned char *texels = stbi_load(path.c_str(), &width_, &height_, &channels, stb_format); // todo: maybe make picking channel type up to caller
		size_ = width_ * height_ * 4; // todo: change depending on passed in channel size
		this->depth_ = 1;

		VV_ASSERT(texels != NULL, "STB failed to load image");

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


	void VulkanImage::createDepthAttachment(VulkanDevice *device, VkExtent2D extent, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		VV_ASSERT(device != VK_NULL_HANDLE, "VulkanDevice not present");
		this->device_ = device;
		this->format = format;
		this->aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
		this->image_view_type = VK_IMAGE_VIEW_TYPE_2D;
		this->path = path;
		this->width_ = extent.width;
		this->height_ = extent.height;
		this->depth_ = 1;

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

		allocateMemory(VK_IMAGE_TYPE_2D, this->format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory_);

		if (hasStencilComponent())
			this->aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

		transformImageLayout(image, this->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}


	void VulkanImage::shutDown()
	{
		if (staging_image_ != VK_NULL_HANDLE)
			vkDestroyImage(device_->logical_device, staging_image_, nullptr);
		if (staging_memory_ != VK_NULL_HANDLE)
			vkFreeMemory(device_->logical_device, staging_memory_, nullptr);
		
		vkDestroyImage(device_->logical_device, image, nullptr);
		vkFreeMemory(device_->logical_device, image_memory_, nullptr);
	}


	void VulkanImage::transferToDevice()
	{
		transformImageLayout(staging_image_, format, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		transformImageLayout(image, format, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        auto command_pool_used = device_->command_pools["graphics"];

        // use transfer queue if available
        if (device_->command_pools.count("transfer") > 0)
            command_pool_used = device_->command_pools["transfer"];

		auto command_buffer = util::beginSingleUseCommand(device_->logical_device, command_pool_used);

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

        if (device_->transfer_family_index != -1)
            util::endSingleUseCommand(device_->logical_device, command_pool_used, command_buffer, device_->transfer_queue);
        else
		    util::endSingleUseCommand(device_->logical_device, command_pool_used, command_buffer, device_->graphics_queue);

		transformImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}


	bool VulkanImage::hasStencilComponent()
	{
		return (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);
	}

	///////////////////////////////////////////////////////////////////////////////////////////// Private

	void VulkanImage::allocateMemory(VkImageType image_type, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkImage &image, VkDeviceMemory &memory)
	{
		VkImageCreateInfo image_create_info = {};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.flags = 0;
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
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT; // other options can be used to help with sparse 3d texture data

        if (device_->transfer_family_index != -1)
        {
            image_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            image_create_info.queueFamilyIndexCount = 2;
            std::array<uint32_t, 2> queue_family_indices = {
                static_cast<uint32_t>(device_->graphics_family_index),
                static_cast<uint32_t>(device_->transfer_family_index)
            };
            image_create_info.pQueueFamilyIndices = queue_family_indices.data();
        }
        else
        {
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.queueFamilyIndexCount = 1;
            uint32_t queue_index = static_cast<uint32_t>(device_->graphics_family_index);
            image_create_info.pQueueFamilyIndices = &queue_index;
        }

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

		VV_CHECK_SUCCESS(vkAllocateMemory(device_->logical_device, &memory_allocate_info, nullptr, &memory));
		vkBindImageMemory(device_->logical_device, image, memory, 0);
	}


	void VulkanImage::transformImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{
        auto command_pool_used = device_->command_pools["graphics"];

        // use transfer queue if available
        if (device_->command_pools.count("transfer") > 0)
            command_pool_used = device_->command_pools["transfer"];

		auto command_buffer = util::beginSingleUseCommand(device_->logical_device, command_pool_used);

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

        if (device_->transfer_family_index != -1)
            util::endSingleUseCommand(device_->logical_device, command_pool_used, command_buffer, device_->transfer_queue);
        else
		    util::endSingleUseCommand(device_->logical_device, command_pool_used, command_buffer, device_->graphics_queue);
	}
}