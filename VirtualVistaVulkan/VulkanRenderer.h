
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>
#include <array>

//#ifdef _DEBUG
//    #define ENABLE_VULKAN_RENDERDOC_CAPTURE 1
//#endif

#include "GLFWWindow.h"
#include "VulkanSwapChain.h"
#include "VulkanImageView.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "Scene.h"
#include "Material.h"
#include "ModelManager.h"
#include "Mesh.h"
#include "Shader.h"
#include "Utils.h"

namespace vv
{
	class VulkanRenderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		/*
		 * Initialize all necessary Vulkan internals.
		 */
		void create();

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
         * Returns a scene object used to store entities with physical presence.
         */
        Scene* getScene() const;

		/*
		 * Returns whether the renderer should stop execution.
		 */
		bool shouldStop();

	private:
		GLFWWindow *window_						    = nullptr;
		VkInstance instance_					    = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT debug_callback_    = VK_NULL_HANDLE;
		VulkanDevice* physical_device_              = nullptr;
		VulkanSwapChain *swap_chain_;

		std::vector<VkFramebuffer> frame_buffers_;

		VkSemaphore image_ready_semaphore_          = VK_NULL_HANDLE;
		VkSemaphore rendering_complete_semaphore_   = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> command_buffers_;

		Shader *shader_;
		VulkanPipeline *pipeline_;
		VulkanRenderPass *render_pass_              = nullptr;

        Scene *scene_;

        std::vector<const char*> used_validation_layers_ = { "VK_LAYER_LUNARG_standard_validation" };
		const std::vector<const char*> used_instance_extensions_ = { VK_EXT_DEBUG_REPORT_EXTENSION_NAME };

		/*
		 * Creates the main Vulkan instance upon which the renderer rests.
		 */
		void createVulkanInstance();

        /*
		 * Creates device handles for any GPUs found with Vulkan support.
	 	 */
		void createVulkanDevices();

		/*
		 * Creates a Vulkan surface for generically communicating between Vulkan and the system window api.
		 */
		void createVulkanSurface();

		/*
		 * Sets up all debug callbacks that handle accepting and printing validation layer reports.
		 */
		void setupDebugCallback();

		/*
		 * Creates Vulkan FrameBuffer objects that encapsulate all of the Vulkan image textures in the swap chain for storing rendered frames.
		 */
		void createFrameBuffers();

		/*
		 * Creates a list of executable commands that will be sent to a command pool.
		 * These commands range from memory management calls, to binding framebuffers, to draw commands.
         * note: VkCommandBuffers can be initialized at the beginning of initialization time.
         *       Simply point them to a range of vertex and index data that update during runtime.
		 */
		void createCommandBuffers();

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
	};
}

#endif // VIRTUALVISTA_VULKANRENDERER_H