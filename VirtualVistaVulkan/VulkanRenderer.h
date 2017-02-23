
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>
#include <array>

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
		 * note: this needs to be called at application termination to ensure that all corresponding Vulkan pieces
         *       are destroyed in the proper order. This is needed because of C++'s lack of destructor guarantees.
		 */
		void shutDown();

		/*
		 * Submits all command buffers to the various vulkan queues for execution.
		 */
		void run(float delta_time);

        /*
         * Signals the renderer to start recording vulkan command buffers once the scene has been properly populated.
         */
        void recordCommandBuffers();

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

		VulkanRenderPass *render_pass_              = nullptr;
		std::vector<VkCommandBuffer> command_buffers_;

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
		 * Sets up all debug callbacks that handle accepting and printing validation layer reports.
		 */
		void setupDebugCallback();

		/*
		 * Creates Vulkan FrameBuffer objects that encapsulate all of the Vulkan image textures in the swap chain for storing rendered frames.
		 */
		void createFrameBuffers();

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