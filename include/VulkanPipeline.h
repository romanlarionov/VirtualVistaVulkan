
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
		VulkanDevice *m_device;
        VulkanRenderPass *m_render_pass;
        std::vector<VulkanShaderModule> m_shader_modules;

        bool m_is_graphics_pipeline = true;

        std::vector<VkPipelineShaderStageCreateInfo> m_shader_state_create_info;
        VkPipelineVertexInputStateCreateInfo m_vertex_input_state_create_info        = {};
        VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state_create_info    = {};
        VkPipelineViewportStateCreateInfo m_viewport_state_create_info               = {};
        VkPipelineRasterizationStateCreateInfo m_rasterization_state_create_info     = {};
        VkPipelineDynamicStateCreateInfo m_dynamic_state_create_info                 = {};
        VkPipelineTessellationStateCreateInfo m_tessellation_state_create_info       = {};
        VkPipelineMultisampleStateCreateInfo m_multisample_state_create_info         = {};
        VkPipelineDepthStencilStateCreateInfo m_depth_stencil_state_create_info      = {};
        VkPipelineColorBlendStateCreateInfo m_color_blend_state_create_info          = {};

	};
}

#endif // VIRTUALVISTA_VULKANPIPELINE_H