
#ifndef VIRTUALVISTA_VULKANPIPELINE_H
#define VIRTUALVISTA_VULKANPIPELINE_H

#include <vector>

#include "VulkanShaderModule.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"

namespace vv
{
	class VulkanPipeline
	{
	public:
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

		VulkanPipeline();
		~VulkanPipeline();

		/*
		 * Creates a graphics pipeline abstraction.
		 */
	    void createGraphicsPipeline(VulkanDevice *device, VkPipelineLayout pipeline_layout, VulkanRenderPass *render_pass);

        /*
		 * Creates a compute pipeline abstraction.
		 */
	    void createComputePipeline(VulkanDevice *device, VkPipelineLayout pipeline_layout, VulkanRenderPass *render_pass);

        /*
         *
         */
        void commitGraphicsPipeline();

        /*
         *
         */
        void commitComputePipeline();

		/*
		 *
		 */
		void shutDown();

		/*
		 * Activates this pipeline for use during command buffer recording.
		 */
		void bind(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point) const;

        /*
         *
         */
        bool addShaderStage(VulkanShaderModule &shader_module);

        /*
         *
         */
        bool addRasterizationState(VkFrontFace front_face);

        /*
         *
         */
        bool addVertexInputState();

        /*
         *
         */
        bool addViewportState();

        /*
         *
         */
        bool addInputAssemblyState();

        /*
         *
         */
        bool addMultisampleState();

        /*
         *
         */
        bool addDepthStencilState(VkBool32 depth_test_enable, VkBool32 depth_write_enable);

        /*
         *
         */
        bool addColorBlendState();

        /*
         *
         */
        bool addDynamicState();

        /*
         *
         */
        bool addTessellationState();


	private:
		VulkanDevice *_device;
        VulkanRenderPass *_render_pass;
        std::vector<VulkanShaderModule> _shader_modules;

        bool _is_graphics_pipeline = true;

        std::vector<VkPipelineShaderStageCreateInfo> _shader_state_create_info;
        VkPipelineVertexInputStateCreateInfo _vertex_input_state_create_info        = {};
        VkPipelineInputAssemblyStateCreateInfo _input_assembly_state_create_info    = {};
        VkPipelineViewportStateCreateInfo _viewport_state_create_info               = {};
        VkPipelineRasterizationStateCreateInfo _rasterization_state_create_info     = {};
        VkPipelineDynamicStateCreateInfo _dynamic_state_create_info                 = {};
        VkPipelineTessellationStateCreateInfo _tessellation_state_create_info       = {};
        VkPipelineMultisampleStateCreateInfo _multisample_state_create_info         = {};
        VkPipelineDepthStencilStateCreateInfo _depth_stencil_state_create_info      = {};
        VkPipelineColorBlendStateCreateInfo _color_blend_state_create_info          = {};

	};
}

#endif // VIRTUALVISTA_VULKANPIPELINE_H