
#include "VulkanImageView.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanImageView::VulkanImageView()
	{
	}


	VulkanImageView::~VulkanImageView()
	{
	}

	void VulkanImageView::create(VulkanDevice *device, VulkanImage *image, VkImageViewType image_view_type, uint32_t base_mip_level)
	{
		_device = device;
		_image = image;
        type = image_view_type;

		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.flags = VK_NULL_HANDLE;
		image_view_create_info.image = image->image;
		image_view_create_info.viewType = image_view_type;
		image_view_create_info.format = image->format;

		// todo: make general
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		image_view_create_info.subresourceRange.aspectMask = image->aspect_flags;
        image_view_create_info.subresourceRange.baseMipLevel = base_mip_level;
		image_view_create_info.subresourceRange.levelCount = image->mip_levels;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = image->array_layers;

		VV_CHECK_SUCCESS(vkCreateImageView(device->logical_device, &image_view_create_info, nullptr, &image_view));
	}


	void VulkanImageView::shutDown()
	{
        vkDestroyImageView(_device->logical_device, image_view, nullptr);
	}
}