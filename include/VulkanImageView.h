
#ifndef VIRTUALVISTA_VULKANIMAGEVIEW_H
#define VIRTUALVISTA_VULKANIMAGEVIEW_H

#include "VulkanDevice.h"
#include "VulkanImage.h"

namespace vv
{
	class VulkanImageView
	{
	public:
		VkImageView image_view;
        VkImageViewType type;

		VulkanImageView();
		~VulkanImageView();

		/*
		 * Creates an image view for the application to interact with.
         *
         * note: This class does not maintain ownership over VulkanImages.
         *       They must be manually deleted outside of this class.
		 */
		void create(VulkanDevice *device, VulkanImage *image, VkImageViewType image_view_type);

		/*
		 *
		 */
		void shutDown();

	private:
		VulkanDevice *_device;
		VulkanImage *_image;

	};
}

#endif // VIRTUALVISTA_VULKANIMAGEVIEW_H