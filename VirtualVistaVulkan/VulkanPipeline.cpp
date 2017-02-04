
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


	void VulkanPipeline::create(VulkanDevice *device, Shader *shader, VkPipelineLayout pipeline_layout,
                                VulkanRenderPass *render_pass, bool depth_test_enable, bool depth_write_enable)
	{
		device_ = device;

		VkPipelineShaderStageCreateInfo vert_shader_create_info = {};
		vert_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

		vert_shader_create_info.module = shader->vert_module;
		vert_shader_create_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_create_info = {};
		frag_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		frag_shader_create_info.module = shader->frag_module;
		frag_shader_create_info.pName = "main";

		std::array<VkPipelineShaderStageCreateInfo, 2> shaders = { vert_shader_create_info, frag_shader_create_info };

		// Fixed Function Pipeline Layout
		VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
		vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state_create_info.flags = 0;
		vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
		vertex_input_state_create_info.vertexAttributeDescriptionCount = (uint32_t)Vertex::getAttributeDescriptions().size();
		vertex_input_state_create_info.pVertexBindingDescriptions = &Vertex::getBindingDesciption();
		vertex_input_state_create_info.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
		input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_create_info.flags = 0;
		input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // render triangles
		input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

		// todo: viewport should be dynamic. figure out how to update pipeline when needed. probably has to do with dynamic state settings.
		VkViewport viewport = {};
		viewport.width = (float)Settings::inst()->getWindowWidth();
		viewport.height = (float)Settings::inst()->getWindowHeight();
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent.width  = (uint32_t)Settings::inst()->getWindowWidth();
		scissor.extent.height = (uint32_t)Settings::inst()->getWindowHeight();

		VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
		viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_create_info.flags = 0;
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.scissorCount = 1;
		viewport_state_create_info.pViewports = &viewport;
		viewport_state_create_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
		rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create_info.depthClampEnable = VK_FALSE; // clamp geometry within clip space
		rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE; // discard geometry
		rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL; // create fragments from the inside of a polygon
		rasterization_state_create_info.lineWidth = 1.0f;
		rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT; // cull the back of polygons from rendering
		rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // order of vertices
		rasterization_state_create_info.depthBiasEnable = VK_FALSE; // all stuff for shadow mapping? look into it
		rasterization_state_create_info.depthBiasClamp = 0.0f;
		rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
		rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

		// todo: add anti-aliasing settings support
		VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
		multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_create_info.flags = 0;
		multisample_state_create_info.sampleShadingEnable = VK_FALSE;
		multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_create_info.minSampleShading = 1.0f;
		multisample_state_create_info.pSampleMask = nullptr;
		multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_state_create_info.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {};
		depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_create_info.flags = 0;
		depth_stencil_state_create_info.depthTestEnable = depth_test_enable;
		depth_stencil_state_create_info.depthWriteEnable = depth_write_enable;
		depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_create_info.minDepthBounds = 0.0f;
		depth_stencil_state_create_info.maxDepthBounds = 1.0f;
		depth_stencil_state_create_info.stencilTestEnable = VK_FALSE; // dont want to do any cutting of the image currently.
		depth_stencil_state_create_info.front = {};
		depth_stencil_state_create_info.back = {};

		// todo: for some reason, if this is activated the output color is overridden
		// This along with color blend create info specify alpha blending operations
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment_state.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
		color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_create_info.logicOpEnable = VK_FALSE;
		color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_state_create_info.attachmentCount = 1;
		color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
		color_blend_state_create_info.blendConstants[0] = 0.0f;
		color_blend_state_create_info.blendConstants[1] = 0.0f;
		color_blend_state_create_info.blendConstants[2] = 0.0f;
		color_blend_state_create_info.blendConstants[3] = 0.0f;

		// add enum values here for more dynamic pipeline state changes!! 
		/*std::array<VkDynamicState, 2> dynamic_pipeline_settings = { VK_DYNAMIC_STATE_VIEWPORT };

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.flags = 0;
		dynamic_state_create_info.dynamicStateCount = (uint32_t)dynamic_pipeline_settings.size();
		dynamic_state_create_info.pDynamicStates = dynamic_pipeline_settings.data();*/

		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
		graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.flags = 0;
		graphics_pipeline_create_info.stageCount = 2; // vert & frag shader
		graphics_pipeline_create_info.pStages = shaders.data();
		graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
		graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
        graphics_pipeline_create_info.pDynamicState = VK_NULL_HANDLE;//&dynamic_state_create_info;
        graphics_pipeline_create_info.pTessellationState = VK_NULL_HANDLE;
		graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
		graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
		graphics_pipeline_create_info.layout = pipeline_layout;
		graphics_pipeline_create_info.renderPass = render_pass->render_pass;
		graphics_pipeline_create_info.subpass = 0; // index of render_pass that this pipeline will be used with
		graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE; // used for creating new pipeline from existing one.

		// todo: this call can create multiple pipelines with a single call. utilize to improve performance.
		// info: the null handle here specifies a VkPipelineCache that can be used to store pipeline creation info after a pipeline's deletion.
		VV_CHECK_SUCCESS(vkCreateGraphicsPipelines(device_->logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline));
	}


	void VulkanPipeline::shutDown()
	{
		if (pipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(device_->logical_device, pipeline, nullptr);
	}


	void VulkanPipeline::bind(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point) const
	{
		vkCmdBindPipeline(command_buffer, bind_point, pipeline);
	}

	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
