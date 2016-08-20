
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>

#include "Renderer.h"
#include "VulkanRendererHelper.h"
#include "Utils.h"

namespace vv
{
	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		/*
		 * Render pass function. Executed in engine main loop.
		 */
		void run();

		/*
		 * Returns whether the renderer should stop execution.
		 */
		bool shouldStop();

	private:
#ifdef _DEBUG
		const bool enable_validation_layers_ = true;
#else
		const bool enable_validation_layers_ = false;
#endif

		GLFWWindow *window_ = nullptr;
		VkInstance instance_;
		VkDebugReportCallbackEXT debug_callback_;

		VkSurfaceKHR surface_;
		VulkanSurfaceDetailsHandle surface_settings_;
		VkSwapchainKHR swap_chain_;
		VkExtent2D swap_chain_extent_;
		VkFormat swap_chain_format_;
		std::vector<VkImage> swap_chain_images_;

		std::vector<VulkanPhysicalDeviceHandle> physical_devices_;
		VkDevice logical_device_;
		VkQueue graphics_queue_;

		const std::vector<const char*> used_validation_layers_ = { "VK_LAYER_LUNARG_standard_validation" };
		const std::vector<const char*> used_instance_extensions_ = { VK_EXT_DEBUG_REPORT_EXTENSION_NAME };

		/*
		 * Creates the main Vulkan instance upon which the renderer rests.
		 */
		void createVulkanInstance();

		/*
		 * Creates a window to render to.
		 */	
		void createWindow();

		/*
		 * Creates a Vulkan surface for generically communicating between Vulkan and the system window api.
		 */
		void createVulkanSurface();

		/*
		 * Sets up all debug callbacks that handle accepting and printing validation layer reports.
		 */
		void setupDebugCallback();

		/*
		 * Creates physical devices handles for any gpus found with Vulkan support.
	 	 */
		void createVulkanPhysicalDevices();

		/*
		 * Creates a logical abstraction for Vulkan device which can be used to submit commands to.
		 */
		void createVulkanLogicalDevices();

		/*
		 * Creates the abstraction for the Vulkan swap chain.
		 */
		void createVulkanSwapChain();

		/*
		 * Returns back a formatted list of all extensions used by the system.
		 */
		std::vector<const char*> getRequiredExtensions();

		/*
		 * Checks to see if the required 3rd party Vulkan extensions are available on the current system
		 */
		bool checkInstanceExtensionSupport();

		/* 
		 * Checks to see if the validation layers that were requested are available on the current system
		 * FOR DEBUGGING PURPOSES ONLY 
		 */
		bool checkValidationLayerSupport();

		/*
		 * Picks the best image format to store the rendered frames in out of the surface's supported formats.
		 * todo: maybe use settings to determine which format to choose. For now, only accept the best possible format.
		 */
		VkSurfaceFormatKHR chooseSwapSurfaceFormat();

		/*
		 * Picks the best form of image buffering. Used for establishing things like vertical sync.
		 * todo: definitely defer to settings, as this is a very common setting in most applications. 
		 */
		VkPresentModeKHR chooseSwapSurfacePresentMode();

		/*
		 * Picks what resolution the images in the swap chain should be.
		 */
		VkExtent2D chooseSwapSurfaceExtent();
	};
}

#endif // VIRTUALVISTA_VULKANRENDERER_H