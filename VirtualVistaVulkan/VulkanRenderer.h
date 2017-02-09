
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
		 * This needs to be called at application termination to ensure that all corresponding Vulkan
		 * pieces are destroyed in the proper order. This is needed because of C++'s lack of destructor guarantees.
		 */
		void shutDown();

		/*
		 * Render pass function. Executed in engine main loop.
		 */
		void run();

        /*
         *
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

		// global setting data for shaders
        const uint32_t MAX_DESCRIPTOR_SETS          = 100;
        const uint32_t MAX_UNIFORM_BUFFERS          = 100;
        const uint32_t MAX_COMBINED_IMAGE_SAMPLERS  = 100;
		VkDescriptorPool descriptor_pool_           = VK_NULL_HANDLE;

        // todo: remove
        VulkanDescriptorSetLayout model_descriptor_set_layout_;

        // stores info every shader is required to use (i.e. matrices)
        VulkanDescriptorSetLayout general_descriptor_set_layout_;
		VkDescriptorSet general_descriptor_set_ = VK_NULL_HANDLE;

        // stores light info every shader is required to use
        VulkanDescriptorSetLayout light_descriptor_set_layout_;

        std::vector<MaterialTemplate *> material_templates_;

        Scene *scene_;
        //ModelManager *model_manager_;
        //std::vector<Model> models_;

        UniformBufferObject ubo_;
		VulkanBuffer *uniform_buffer_;
		VkSampler sampler_ = VK_NULL_HANDLE;

		//Mesh *mesh_;
		//VulkanBuffer *vertex_buffer_;
		//VulkanBuffer *index_buffer_;

		// texture aka remove
		//VulkanImage *texture_image_;
		//VulkanImageView *texture_image_view_;

		const std::vector<const char*> used_validation_layers_ = { "VK_LAYER_LUNARG_standard_validation" };
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
         * Loads possible material types from file, parses descriptor set requirements from it's shader,
         * and allocates unique shader/pipeline per template.
         */
        void createMaterialTemplates();

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
		 * Specify to Vulkan a set of descriptors for global resources that will be used, i.e. uniforms.
		 * The VkDescriptorSetLayout object is used as a template for a type of descriptor set that can be made.
		 */
		void createDescriptorSetLayout();

		/*
		 * The Descriptor Set stores all info of a particular descriptor set in use.
		 * This can be a uniform buffer object, a texture, or a texel image view.
		 * The word "descriptor" refers to the term "binding" in the shader for passing in uniform type information.
		 */
		//void createDescriptorSet();
        void createGeneralDescriptorSet();

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