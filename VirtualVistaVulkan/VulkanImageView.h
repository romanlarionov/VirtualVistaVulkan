
#ifndef VIRTUALVISTA_VULKANIMAGEVIEW_H
#define VIRTUALVISTA_VULKANIMAGEVIEW_H

#include <vulkan\vulkan.h>

#include "Utils.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

namespace vv
{
	class VulkanImageView
	{
	public:
		VkImageView image_view;

		VulkanImageView() {}

		~VulkanImageView()
		{
			if (image_view)
				vkDestroyImageView(device_->logical_device, image_view, nullptr);
		}

		/*
		 * Creates an image view for the application to interact with.
		 * todo: consider adding functionality of getting image settings here and constructing the image from this class.
		 * todo: consider removing this class and place this code along with image creation to texture class along with sampler.
		 */
		void create(VulkanDevice *device, VulkanImage *image)
		{
			VV_ASSERT(device, "VulkanDevice not present");
			device_ = device;
			image_ = image;

			VkImageViewCreateInfo image_view_create_info = {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.flags = VK_NULL_HANDLE;
			image_view_create_info.image = image->image;
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = image->format;

			// todo: make general
			image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;

			VV_CHECK_SUCCESS(vkCreateImageView(device->logical_device, &image_view_create_info, nullptr, &image_view));
		}

	private:
		VulkanDevice *device_;
		VulkanImage *image_;

	};
}

#endif // VIRTUALVISTA_VULKANIMAGEVIEW_H