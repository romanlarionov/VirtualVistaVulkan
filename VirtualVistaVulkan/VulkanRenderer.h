
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>

#include "Shader.h"
#include "Renderer.h"
#include "VulkanDevice.h"
#include "Utils.h"

namespace vv
{
	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		/*
		 * Initialize all necessary Vulkan internals.
		 */
		void init();

		/*
		 * Destroy all Vulkan internals in the correct order.
		 * 
		 * This needs to be called at application termination to ensure that all corresponding Vulkan
		 * pieces are destroyed in the proper order. This is needed because of C++'s lack of destructor guarantees.
		 */
		void shutDown();

		/*
		 * Render pass function. Executed in engine main loop.
		 */
		void run();

		/*
		 * Returns whether the renderer should stop execution.
		 */
		bool shouldStop();

	private:
		GLFWWindow *window_ = nullptr;
		VkInstance instance_;
		VkDebugReportCallbackEXT debug_callback_;
		std::vector<VulkanDevice*> physical_devices_;

		VkSurfaceKHR surface_;
		VulkanSurfaceDetailsHandle surface_settings_;
		VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
		VkExtent2D swap_chain_extent_;
		VkFormat swap_chain_format_;
		std::vector<VkImage> swap_chain_images_;
		std::vector<VkImageView> swap_chain_image_views_;

		Shader *shader_;
		VkPipeline pipeline_;

		// uniform data for shaders
		VkPipelineLayout pipeline_layout_;
		VkRenderPass render_pass_;

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
		 * Creates device handles for any gpus found with Vulkan support.
	 	 */
		void createVulkanDevices();

		/*
		 * Creates the abstraction for the Vulkan swap chain.
		 */
		void createVulkanSwapChain();

		/*
		 * Creates image views that explain to vulkan what the list of images for the swap chain are meant to be.
		 */
		void createVulkanImages();

		/*
		 * Starts initializing graphics computation components and their settings (anti-aliasing, rasterizer, etc) once core Vulkan components are ready.
		 */
		void createGraphicsPipeline();

		/*
		 * Starts creating the framebuffers for display.
		 */
		void createRenderPass();

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