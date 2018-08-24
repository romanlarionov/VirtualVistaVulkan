
#include "VulkanPipeline.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanPipeline::VulkanPipeline()
	{
	}


	VulkanPipeline::~VulkanPipeline()
	{
	}


	void VulkanPipeline::createGraphicsPipeline(VulkanDevice *device, VkPipelineLayout pipeline_layout, VulkanRenderPass *render_pass)
	{
	    m_device = device;
        this->pipeline_layout = pipeline_layout;
        m_render_pass = render_pass;
	}


	void VulkanPipeline::createComputePipeline(VulkanDevice *device, VkPipelineLayout pipeline_layout, VulkanRenderPass *render_pass)
    {
	    m_device = device;
        this->pipeline_layout = pipeline_layout;
        m_render_pass = render_pass;
        m_is_graphics_pipeline = false;
    }

	void VulkanPipeline::shutDown()
	{
        vkDestroyPipeline(m_device->logical_device, pipeline, nullptr);
	}


	void VulkanPipeline::bind(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point) const
	{
        vkCmdBindPipeline(command_buffer, bind_point, pipeline);
	}

    void VulkanPipeline::commitGraphicsPipeline()
    {
        if (!m_is_graphics_pipeline) return;

	    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
	    graphics_pipeline_create_info.sType                 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	    graphics_pipeline_create_info.flags                 = 0;
	    graphics_pipeline_create_info.stageCount            = m_shader_state_create_info.size();
	    graphics_pipeline_create_info.pStages               = m_shader_state_create_info.data();
	    graphics_pipeline_create_info.pVertexInputState     = &m_vertex_input_state_create_info;
	    graphics_pipeline_create_info.pInputAssemblyState   = &m_input_assembly_state_create_info;
	    graphics_pipeline_create_info.pViewportState        = &m_viewport_state_create_info;
	    graphics_pipeline_create_info.pRasterizationState   = &m_rasterization_state_create_info;
        graphics_pipeline_create_info.pDynamicState         = VK_NULL_HANDLE;//&_dynamic_state_create_info;
        graphics_pipeline_create_info.pTessellationState    = VK_NULL_HANDLE;//&_tessellation_state_create_info;
	    graphics_pipeline_create_info.pMultisampleState     = &m_multisample_state_create_info;
	    graphics_pipeline_create_info.pDepthStencilState    = &m_depth_stencil_state_create_info;
	    graphics_pipeline_create_info.pColorBlendState      = &m_color_blend_state_create_info;
	    graphics_pipeline_create_info.layout                = pipeline_layout;
	    graphics_pipeline_create_info.renderPass            = m_render_pass->render_pass;
	    graphics_pipeline_create_info.subpass               = 0; // index of render_pass that this pipeline will be used with
	    graphics_pipeline_create_info.basePipelineHandle    = VK_NULL_HANDLE; // used for creating new pipeline from existing one.

	    // info: the null handle here specifies a VkPipelineCache that can be used to store pipeline creation info after a pipeline's deletion.
	    VV_CHECK_SUCCESS(vkCreateGraphicsPipelines(m_device->logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline));
    }

    void VulkanPipeline::commitComputePipeline()
    {
        if (m_is_graphics_pipeline) return;

        VkComputePipelineCreateInfo compute_pipeline_create_info = {};
        compute_pipeline_create_info.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        compute_pipeline_create_info.flags  = 0;
        compute_pipeline_create_info.stage  = m_shader_state_create_info[0]; // should be set up prior to call
        compute_pipeline_create_info.layout = pipeline_layout;
        compute_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

	    VV_CHECK_SUCCESS(vkCreateComputePipelines(m_device->logical_device, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &pipeline));
    }

    bool VulkanPipeline::addShaderStage(VulkanShaderModule &shader_module)
    {
	    VkPipelineShaderStageCreateInfo shader_create_info = {};
	    shader_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	    shader_create_info.stage  = shader_module.shader_stage;
	    shader_create_info.module = shader_module.shader_module;
	    shader_create_info.pName  = shader_module.entrance_function.c_str();

        m_shader_state_create_info.push_back(shader_create_info);
        return true;
    }

    bool VulkanPipeline::addRasterizationState(VkFrontFace front_face)
    {
        if (!m_is_graphics_pipeline) return false;

	    m_rasterization_state_create_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	    m_rasterization_state_create_info.depthClampEnable           = VK_FALSE; // clamp geometry within clip space
	    m_rasterization_state_create_info.rasterizerDiscardEnable    = VK_FALSE; // discard geometry
	    m_rasterization_state_create_info.polygonMode                = VK_POLYGON_MODE_FILL; // create fragments from the inside of a polygon
	    m_rasterization_state_create_info.lineWidth                  = 1.0f;
	    m_rasterization_state_create_info.cullMode                   = VK_CULL_MODE_BACK_BIT; // cull the back of polygons from rendering
        m_rasterization_state_create_info.frontFace                  = front_face;// VK_FRONT_FACE_CLOCKWISE; // order of vertices
	    m_rasterization_state_create_info.depthBiasEnable            = VK_FALSE; // all stuff for shadow mapping? look into it
	    m_rasterization_state_create_info.depthBiasClamp             = 0.0f;
	    m_rasterization_state_create_info.depthBiasConstantFactor    = 0.0f;
	    m_rasterization_state_create_info.depthBiasSlopeFactor       = 0.0f;

        return true;
    }

    bool VulkanPipeline::addVertexInputState()
    {
        if (!m_is_graphics_pipeline) return false;

	    // Fixed Function Pipeline Layout
	    m_vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	    m_vertex_input_state_create_info.flags                           = 0;
	    m_vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
	    m_vertex_input_state_create_info.vertexAttributeDescriptionCount = 3; // todo: hardcoded for vertices
	    m_vertex_input_state_create_info.pVertexBindingDescriptions      = new VkVertexInputBindingDescription(Vertex::getBindingDesciption());
	    m_vertex_input_state_create_info.pVertexAttributeDescriptions    = Vertex::getAttributeDescriptions();

        return true;
    }

    bool VulkanPipeline::addViewportState()
    {
        if (!m_is_graphics_pipeline) return false;

	    // todo: viewport should be dynamic. figure out how to update pipeline when needed. probably has to do with dynamic state settings.
	    VkViewport viewport = {};
	    viewport.width = static_cast<float>(Settings::inst()->getWindowWidth());
	    viewport.height = static_cast<float>(Settings::inst()->getWindowHeight());
	    viewport.x = 0.0f;
	    viewport.y = 0.0f;
	    viewport.minDepth = 0.0f;
	    viewport.maxDepth = 1.0f;

	    VkRect2D scissor = {};
	    scissor.offset = { 0, 0 };
	    scissor.extent.width  = static_cast<uint32_t>(Settings::inst()->getWindowWidth());
	    scissor.extent.height = static_cast<uint32_t>(Settings::inst()->getWindowHeight());

	    m_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	    m_viewport_state_create_info.flags = 0;
	    m_viewport_state_create_info.viewportCount = 1;
	    m_viewport_state_create_info.scissorCount = 1;
        m_viewport_state_create_info.pViewports = new VkViewport(viewport);
        m_viewport_state_create_info.pScissors = new VkRect2D(scissor);

        return true;
    }

    bool VulkanPipeline::addInputAssemblyState()
    {
        if (!m_is_graphics_pipeline) return false;

	    m_input_assembly_state_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	    m_input_assembly_state_create_info.flags                  = 0;
	    m_input_assembly_state_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // render triangles
	    m_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

        return true;
    }

    bool VulkanPipeline::addMultisampleState()
    {
        if (!m_is_graphics_pipeline) return false;

	    m_multisample_state_create_info.sType                    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	    m_multisample_state_create_info.flags                    = 0;
	    m_multisample_state_create_info.sampleShadingEnable      = VK_FALSE;
	    m_multisample_state_create_info.rasterizationSamples     = VK_SAMPLE_COUNT_1_BIT;
	    m_multisample_state_create_info.minSampleShading         = 1.0f;
	    m_multisample_state_create_info.pSampleMask              = nullptr;
	    m_multisample_state_create_info.alphaToCoverageEnable    = VK_FALSE;
	    m_multisample_state_create_info.alphaToOneEnable         = VK_FALSE;

        return true;
    }

    bool VulkanPipeline::addDepthStencilState(VkBool32 depth_test_enable, VkBool32 depth_write_enable)
    {
        if (!m_is_graphics_pipeline) return false;

		m_depth_stencil_state_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		m_depth_stencil_state_create_info.flags                  = 0;
		m_depth_stencil_state_create_info.depthTestEnable        = depth_test_enable;
		m_depth_stencil_state_create_info.depthWriteEnable       = depth_write_enable;
		m_depth_stencil_state_create_info.depthCompareOp         = VK_COMPARE_OP_LESS;
		m_depth_stencil_state_create_info.depthBoundsTestEnable  = VK_FALSE;
		m_depth_stencil_state_create_info.minDepthBounds         = 0.0f;
		m_depth_stencil_state_create_info.maxDepthBounds         = 1.0f;
		m_depth_stencil_state_create_info.stencilTestEnable      = VK_FALSE; // don't want to do any cutting of the image currently.
		m_depth_stencil_state_create_info.front                  = {};
		m_depth_stencil_state_create_info.back                   = {};

        return true;
    }

    bool VulkanPipeline::addColorBlendState()
    {
        if (!m_is_graphics_pipeline) return false;

	    // todo: for some reason, if this is activated the output color is overridden
	    // This along with color blend create info specify alpha blending operations
	    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
	    color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	    color_blend_attachment_state.blendEnable = VK_FALSE;

	    m_color_blend_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	    m_color_blend_state_create_info.logicOpEnable     = VK_FALSE;
	    m_color_blend_state_create_info.logicOp           = VK_LOGIC_OP_COPY;
	    m_color_blend_state_create_info.attachmentCount   = 1;
	    m_color_blend_state_create_info.pAttachments      = new VkPipelineColorBlendAttachmentState(color_blend_attachment_state);
	    m_color_blend_state_create_info.blendConstants[0] = 0.0f;
	    m_color_blend_state_create_info.blendConstants[1] = 0.0f;
	    m_color_blend_state_create_info.blendConstants[2] = 0.0f;
	    m_color_blend_state_create_info.blendConstants[3] = 0.0f;

        return true;
    }

    bool VulkanPipeline::addDynamicState()
    {
        //if (!_is_graphics_pipeline) return false;
        // add enum values here for more dynamic pipeline state changes!! 
        //std::array<VkDynamicState, 2> dynamic_pipeline_settings = { VK_DYNAMIC_STATE_VIEWPORT };

        //_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        //_dynamic_state_create_info.flags = 0;
        //_dynamic_state_create_info.dynamicStateCount = (uint32_t)dynamic_pipeline_settings.size();
        //_dynamic_state_create_info.pDynamicStates = dynamic_pipeline_settings.data();

        return false;
    }

    bool VulkanPipeline::addTessellationState()
    {
        return false;
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
