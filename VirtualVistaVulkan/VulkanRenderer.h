
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>
#include <array>
#include <glm/glm.hpp>

#include "GLFWWindow.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "Shader.h"
#include "Utils.h"

namespace vv
{
	struct Vertex
	{
	public:
		glm::vec2 position;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDesciption()
		{
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions;

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0; // layout placement
			attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // type
			attribute_descriptions[0].offset = offsetof(Vertex, position); // placement in vertex 

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(Vertex, color);

			return attribute_descriptions;
		}
	};

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

		// uniform data for shaders
		VkPipelineLayout pipeline_layout_;
		VkRenderPass render_pass_;

		VkCommandPool graphics_command_pool_;
		VkCommandPool transfer_command_pool_;
		std::vector<VkCommandBuffer> command_buffers_;

		VkBuffer vertex_buffer_;
		VkDeviceMemory vertex_buffer_memory_;

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
		 * Starts creating the framebuffers for display.
		 */
		void createRenderPass();

		/*
		 * Creates Vulkan FrameBuffer objects that encapsulate all of the Vulkan image textures in the swap chain for storing rendered frames.
		 */
		void createFrameBuffers();

		/*
		 * Creates a Vulkan pool that stores GPU operations such as memory transfers, draw calls, and compute calls.
		 */
		void createCommandPool(int index, VkCommandPool &command_pool);

		/*
		 * Creates a list of executable commands that will be sent to a command pool.
		 * These commands range from memory management calls, to binding framebuffers, to draw commands. 
		 */
		void createCommandBuffers();

		/*
		 * Creates locks for command queue operations to handle their asynchronous execution.
		 */
		void createVulkanSemaphores();

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory);

		void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

		void createVertexBuffers();

		uint32_t findMemoryType(uint32_t filter_type, VkMemoryPropertyFlags memory_property_flags);

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