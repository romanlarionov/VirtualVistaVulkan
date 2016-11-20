
#ifndef VIRTUALVISTA_VULKANIMAGEVIEW_H
#define VIRTUALVISTA_VULKANIMAGEVIEW_H

#include <vulkan\vulkan.h>

#include "VulkanDevice.h"
#include "VulkanImage.h"

namespace vv
{
	class VulkanImageView
	{
	public:
		VkImageView image_view;

		VulkanImageView();
		~VulkanImageView();

		/*
		 * Creates an image view for the application to interact with.
		 * todo: consider adding functionality of getting image settings here and constructing the image from this class.
		 * todo: consider removing this class and place this code along with image creation to texture class along with sampler.
		 */
		void create(VulkanDevice *device, VulkanImage *image);

		/*
		 *
		 */
		void shutDown();

	private:
		VulkanDevice *device_;
		VulkanImage *image_;

	};
}

#endif // VIRTUALVISTA_VULKANIMAGEVIEW_H