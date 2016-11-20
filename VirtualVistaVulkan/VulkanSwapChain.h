
#ifndef VIRTUALVISTA_VULKANSWAPCHAIN_H
#define VIRTUALVISTA_VULKANSWAPCHAIN_H

#include <vector>
#include <algorithm>

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "Utils.h"

#ifdef _WIN32
#define NOMINMAX
#endif

namespace vv
{
	struct VulkanSwapChain
	{
	public:
		VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
		VkExtent2D extent;
		VkFormat format;
		std::vector<VulkanImage*> images;
		std::vector<VulkanImageView*> image_views;

		VulkanSwapChain();
		//{
		//}

		~VulkanSwapChain();
		//{
		//}
	
		/*
		 * Creates the abstraction for the Vulkan swap chain.
		 * todo: this could be altered to allow for on-the-fly graphics configuration without a need for a hard restart.
		 * for now, I'm setting the swap chain to be initialized with whatever the default settings are on renderer initialization.
		 */
		void create(VulkanDevice *device, GLFWWindow *window);
		/*{
			VV_ASSERT(device, "VulkanDevice not present");
			VV_ASSERT(window, "Window not present");

			VkSwapchainKHR old_swap_chain = swap_chain;
			window_ = window;

			VkSurfaceFormatKHR chosen_format = chooseSurfaceFormat(device);
			VkPresentModeKHR chosen_present_mode = chooseSurfacePresentMode(device);
			format = chosen_format.format;

			// Swap Chain Extent
			if (window_->surface_settings[device].surface_capabilities.currentExtent.width == (uint32_t)-1)
			{
				// If the surface size is undefined, the size is set to the size of the images requested.
				extent.width = Settings::inst()->getWindowWidth();
				extent.height = Settings::inst()->getWindowHeight();
			}
			else
			{
				// If the surface size is defined, the swap chain size must match.
				extent = window_->surface_settings[device].surface_capabilities.currentExtent;
				Settings::inst()->setWindowWidth(extent.width);
				Settings::inst()->setWindowHeight(extent.height);
			}

			// Queue length for swap chain. (How many images are kept waiting).
			uint32_t image_count = window_->surface_settings[device].surface_capabilities.minImageCount;
			if ((window_->surface_settings[device].surface_capabilities.maxImageCount > 0) &&
				(image_count > window_->surface_settings[device].surface_capabilities.maxImageCount)) // if 0, maxImageCount doesn't have a limit.
				image_count = window_->surface_settings[device].surface_capabilities.maxImageCount;

			VkSwapchainCreateInfoKHR swap_chain_create_info = {};
			swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swap_chain_create_info.flags = 0;
			swap_chain_create_info.surface = window_->surface;
			swap_chain_create_info.minImageCount = image_count;
			swap_chain_create_info.imageFormat = chosen_format.format;
			swap_chain_create_info.imageColorSpace = chosen_format.colorSpace;
			swap_chain_create_info.imageExtent = extent;
			swap_chain_create_info.imageArrayLayers = 1; // todo: make dynamic. 2 would be for VR support
			swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT <- to do post processing

			if (device->graphics_family_index != device->display_family_index)
			{
				uint32_t queue[] = { (uint32_t)device->graphics_family_index, (uint32_t)(device->display_family_index) };
				swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swap_chain_create_info.queueFamilyIndexCount = 2; // display and graphics use two different queues
				swap_chain_create_info.pQueueFamilyIndices = queue;
			}
			else
			{
				swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swap_chain_create_info.queueFamilyIndexCount = 0;
				swap_chain_create_info.pQueueFamilyIndices = nullptr;
			}

			swap_chain_create_info.preTransform = window_->surface_settings[device].surface_capabilities.currentTransform;
			swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swap_chain_create_info.presentMode = chosen_present_mode;
			swap_chain_create_info.clipped = VK_TRUE;
			swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;
			swap_chain_create_info.oldSwapchain = old_swap_chain;

			// If we've just created a new swap chain, we need to delete the old Vulkan objects.
			if (old_swap_chain != VK_NULL_HANDLE)
			{
				shutDown(device);
				/*for (uint32_t i = 0; i < image_count; ++i)
					vkDestroyImageView(device->logical_device, image_views[i]->image_view, nullptr);

				vkDestroySwapchainKHR(device->logical_device, swap_chain, nullptr);*/
			/*}

			VV_CHECK_SUCCESS(vkCreateSwapchainKHR(device->logical_device, &swap_chain_create_info, nullptr, &swap_chain));
			createVulkanImageViews(device);
		}*/

		/*
		 * Destroys all Vulkan internals in the proper order.
		 */
		void shutDown(VulkanDevice *device);
		/*{
			VV_ASSERT(device, "Vulkan Device not present");
			if (swap_chain != VK_NULL_HANDLE)
			{
				for (std::size_t i = 0; i < image_views.size(); ++i)
					delete image_views[i];
				//	vkDestroyImageView(device->logical_device, image_views[i]->image_view, nullptr);

				vkDestroySwapchainKHR(device->logical_device, swap_chain, nullptr);
			}
		}*/
	
		/*
		 * Call for Vulkan to acquire the next image in the swap chain prior to rendering.
		 */
		VkResult acquireNextImage(VulkanDevice *device, VkSemaphore image_ready_semaphore, uint32_t &image_index);
		//{
		//	return vkAcquireNextImageKHR(device->logical_device, swap_chain, UINT64_MAX, image_ready_semaphore, VK_NULL_HANDLE, &image_index);
		//}

		/*
		 * Queue a loaded swap chain image for rendering.
		 */
		VkResult queuePresent(VkQueue queue, uint32_t &image_index, VkSemaphore wait_semaphore = VK_NULL_HANDLE);
		/*{
			VkPresentInfoKHR present_info = {};
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.waitSemaphoreCount = 1;
			present_info.pWaitSemaphores = &wait_semaphore;

			present_info.swapchainCount = 1;
			present_info.pSwapchains = &swap_chain;
			present_info.pImageIndices = &image_index;

			return vkQueuePresentKHR(queue, &present_info);
		}*/

	private:
		GLFWWindow *window_;

		/*
		 * Creates image views that explain to Vulkan what the list of images for the swap chain are meant to be.
		 */
		void createVulkanImageViews(VulkanDevice *device);
		/*{
			// This is effectively creating a queue of frames to be displayed. 
			// todo: support VR by having multiple lists containing images/image views for each eye.

			uint32_t swap_chain_image_count = 0;
			VV_CHECK_SUCCESS(vkGetSwapchainImagesKHR(device->logical_device, swap_chain, &swap_chain_image_count, nullptr));
			std::vector<VkImage> raw_images(swap_chain_image_count);
			VV_CHECK_SUCCESS(vkGetSwapchainImagesKHR(device->logical_device, swap_chain, &swap_chain_image_count, raw_images.data()));

			images.resize(swap_chain_image_count);
			image_views.resize(swap_chain_image_count);

			for (uint32_t i = 0; i < swap_chain_image_count; ++i)
			{
				// convert to abstracted format
				VulkanImage *curr_image = new VulkanImage();
				curr_image->create(device, format, raw_images[i]);
				images[i] = curr_image;

				VulkanImageView *curr_image_view = new VulkanImageView();
				curr_image_view->create(device, curr_image);
				image_views[i] = curr_image_view;

				/*VkImageViewCreateInfo image_view_create_info = {};
				image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				image_view_create_info.flags = VK_NULL_HANDLE;
				image_view_create_info.image = images[i];
				image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				image_view_create_info.format = format;

				image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

				image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				image_view_create_info.subresourceRange.baseMipLevel = 0;
				image_view_create_info.subresourceRange.levelCount = 1;
				image_view_create_info.subresourceRange.baseArrayLayer = 0;
				image_view_create_info.subresourceRange.layerCount = 1;

				VkImageView image_view = {};
				VV_CHECK_SUCCESS(vkCreateImageView(device->logical_device, &image_view_create_info, nullptr, &image_view));
				image_views[i] = image_view;*/
			/*}
		}*/

		/*
		 * Picks the best image format to store the rendered frames in out of the surface's supported formats.
		 * todo: maybe use settings to determine which format to choose. For now, only accept the best possible format.
		 */
		VkSurfaceFormatKHR chooseSurfaceFormat(VulkanDevice *device);
		/*{
			// The case when the surface has no preferred format. I choose to use standard sRGB for storage and 32 bit linear for computation.
			if ((window_->surface_settings[device].available_surface_formats.size() == 1) && window_->surface_settings[device].available_surface_formats[0].format == VK_FORMAT_UNDEFINED)
				return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

			// Try to find the format specified above from the list of supported formats.
			for (auto &format : window_->surface_settings[device].available_surface_formats)
				if ((format.format == VK_FORMAT_B8G8R8A8_UNORM) && (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
					return format;

			// If the desired format cannot be found, pick the first available one.
			if (!window_->surface_settings[device].available_surface_formats.empty())
				return window_->surface_settings[device].available_surface_formats[0];

			return {};
		}*/

		/*
		 * Picks the best form of image buffering. Used for establishing things like vertical sync.
		 * todo: definitely defer to settings, as this is a very common setting in most applications. 
		 */
		VkPresentModeKHR chooseSurfacePresentMode(VulkanDevice *device);
		/*{
			for (const auto &mode : window_->surface_settings[device].available_surface_present_modes)
				if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
					return mode;

			return VK_PRESENT_MODE_FIFO_KHR;
		}*/
	};
}

#endif // VIRTUALVISTA_VULKANSWAPCHAIN_H