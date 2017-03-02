
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
         *
         * note: This class does not maintain ownership over VulkanImages.
         *       They must be manually deleted outside of this class.
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