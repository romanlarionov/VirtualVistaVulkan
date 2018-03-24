
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
		std::vector<VulkanImage*> color_images;
		std::vector<VulkanImageView*> color_image_views;
		VulkanImage *depth_image;
		VulkanImageView *depth_image_view;

		VulkanSwapChain();
		~VulkanSwapChain();
	
		/*
		 * Creates the abstraction for the Vulkan swap chain.
		 * todo: this could be altered to allow for on-the-fly graphics configuration without a need for a hard restart.
		 * for now, I'm setting the swap chain to be initialized with whatever the default settings are on renderer initialization.
		 */
		void create(VulkanDevice *device, GLFWWindow *window);

		/*
		 * Destroys all Vulkan internals in the proper order.
		 */
		void shutDown(VulkanDevice *device);
	
		/*
		 * Call for Vulkan to acquire the next image in the swap chain prior to rendering.
		 */
		void acquireNextImage(VulkanDevice *device, VkSemaphore image_ready_semaphore, uint32_t &image_index);

		/*
		 * Queue a loaded swap chain image for rendering.
		 */
		void present(VkQueue queue, uint32_t &image_index, VkSemaphore wait_semaphore = VK_NULL_HANDLE);

	private:
		GLFWWindow *window_;

		/*
		 * Creates image views that explain to Vulkan what the list of images for the swap chain are meant to be.
		 */
		void createVulkanImageViews(VulkanDevice *device);

		/*
		 * Picks the best image format to store the rendered frames in out of the surface's supported formats.
		 * todo: maybe use settings to determine which format to choose. For now, only accept the best possible format.
		 */
		VkSurfaceFormatKHR chooseSurfaceFormat(VulkanDevice *device);

		/*
		 * Picks the best form of image buffering. Used for establishing things like vertical sync.
		 * todo: definitely defer to settings, as this is a very common setting in most applications. 
		 */
		VkPresentModeKHR chooseSurfacePresentMode(VulkanDevice *device);
	};
}

#endif // VIRTUALVISTA_VULKANSWAPCHAIN_H