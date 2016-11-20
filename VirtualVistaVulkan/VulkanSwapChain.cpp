
#include "VulkanSwapChain.h"

#ifdef _WIN32
#define NOMINMAX
#endif

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanSwapChain::VulkanSwapChain()
	{
	}


	VulkanSwapChain::~VulkanSwapChain()
	{
	}
	
	
	void VulkanSwapChain::create(VulkanDevice *device, GLFWWindow *window)
	{
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
			shutDown(device);

		VV_CHECK_SUCCESS(vkCreateSwapchainKHR(device->logical_device, &swap_chain_create_info, nullptr, &swap_chain));
		createVulkanImageViews(device);
	}


	void VulkanSwapChain::shutDown(VulkanDevice *device)
	{
		VV_ASSERT(device, "Vulkan Device not present");
		if (swap_chain != VK_NULL_HANDLE)
		{
			for (std::size_t i = 0; i < image_views.size(); ++i)
			{
				image_views[i]->shutDown();
				delete image_views[i];
			}

			vkDestroySwapchainKHR(device->logical_device, swap_chain, nullptr);
		}
	}
	
	
	VkResult VulkanSwapChain::acquireNextImage(VulkanDevice *device, VkSemaphore image_ready_semaphore, uint32_t &image_index)
	{
		return vkAcquireNextImageKHR(device->logical_device, swap_chain, UINT64_MAX, image_ready_semaphore, VK_NULL_HANDLE, &image_index);
	}

	
	VkResult VulkanSwapChain::queuePresent(VkQueue queue, uint32_t &image_index, VkSemaphore wait_semaphore)
	{
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &wait_semaphore;

		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swap_chain;
		present_info.pImageIndices = &image_index;

		return vkQueuePresentKHR(queue, &present_info);
	}


    ///////////////////////////////////////////////////////////////////////////////////////////// Public
	void VulkanSwapChain::createVulkanImageViews(VulkanDevice *device)
	{
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
			curr_image->create(raw_images[i], device, format);
			images[i] = curr_image;

			VulkanImageView *curr_image_view = new VulkanImageView();
			curr_image_view->create(device, curr_image);
			image_views[i] = curr_image_view;
		}
	}


	VkSurfaceFormatKHR VulkanSwapChain::chooseSurfaceFormat(VulkanDevice *device)
	{
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
	}


	VkPresentModeKHR VulkanSwapChain::chooseSurfacePresentMode(VulkanDevice *device)
	{
		for (const auto &mode : window_->surface_settings[device].available_surface_present_modes)
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;

		return VK_PRESENT_MODE_FIFO_KHR;
	}
}
