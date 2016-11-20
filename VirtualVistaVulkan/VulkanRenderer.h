
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>
#include <array>

#include "GLFWWindow.h"
#include "VulkanSwapChain.h"
#include "VulkanImageView.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
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
		VkInstance instance_ = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT debug_callback_;
		std::vector<VulkanDevice*> physical_devices_;
		VulkanSwapChain *swap_chain_;

		std::vector<VkFramebuffer> frame_buffers_;

		Shader *shader_;
		VkPipeline pipeline_;

		// data for shaders
		VkDescriptorSetLayout descriptor_set_layout_; // todo: move to model class
		VkDescriptorPool descriptor_pool_;
		VkDescriptorSet descriptor_set_;
		VkPipelineLayout pipeline_layout_;
		VkRenderPass render_pass_;

		std::vector<VkCommandBuffer> command_buffers_;

		// todo: remove as this is not very general
		VulkanBuffer *vertex_buffer_;
		VulkanBuffer *index_buffer_;
		UniformBufferObject ubo_;
		VulkanBuffer *uniform_buffer_;

		// texture aka remove
		VulkanImage *texture_image_;
		VulkanImageView *texture_image_view_;
		VkSampler sampler_;

		VkSemaphore image_ready_semaphore_;
		VkSemaphore rendering_complete_semaphore_;

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
		 * Creates device handles for any GPUs found with Vulkan support.
	 	 */
		void createVulkanDevices();

		/*
		 * Creates the abstraction for the Vulkan swap chain.
		 */
		void createVulkanSwapChain();

		/*
		 * Starts initializing graphics computation components and their settings (anti-aliasing, rasterizer, etc) once core Vulkan components are ready.
		 */
		void createGraphicsPipeline();

		/*
		 * Specify to Vulkan a set of descriptors for global resources that will be used, i.e. uniforms.
		 * The VkDescriptorSetLayout object is used as a template for a type of descriptor set that can be made.
		 */
		void createDescriptorSetLayout();

		/*
		 * The Descriptor Set stores all info of a particular descriptor set in use.
		 * This can be a uniform buffer object, a texture, or a texel image view.
		 * The word "descriptor" refers to the term "binding" in the shader for passing in uniform type information.
		 */
		void createDescriptorSet();

		/*
		 * The Descriptor Pool manages Descriptor Sets. As a result, no manual deletion for descriptor sets is needed. The pool will handle it.
		 */
		void createDescriptorPool();

		/*
		 * Sampler is what allows for interpolation of texture data on the GPU.
		 * Doesn't hold texture data, just the processes that should be acted on it.
		 */
		void createSampler();

		/*
		 * Starts creating the framebuffers for display.
		 */
		void createRenderPass();

		/*
		 * Creates Vulkan FrameBuffer objects that encapsulate all of the Vulkan image textures in the swap chain for storing rendered frames.
		 */
		void createFrameBuffers();

		/*
		 * Creates a list of executable commands that will be sent to a command pool.
		 * These commands range from memory management calls, to binding framebuffers, to draw commands. 
		 */
		void createCommandBuffers();

		/*
		 * Creates locks for command queue operations to handle their asynchronous execution.
		 */
		void createVulkanSemaphores();

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