
#include "VulkanRenderPass.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanRenderPass::VulkanRenderPass() :
		has_been_created(false)
	{
	}


	VulkanRenderPass::~VulkanRenderPass()
	{
	}


	void VulkanRenderPass::create(VulkanDevice *device, VulkanSwapChain *swap_chain)
	{
		if (has_been_created) return;

		VV_ASSERT(device != nullptr, "Vulkan Device is NULL");
		_device = device;

		VkAttachmentDescription color_attachment_description = {};
		color_attachment_description.flags = 0;
		color_attachment_description.format = swap_chain->format;
		color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT; // change for multi-sampling support
		color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clears the image at the beginning of each render
		color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // save the image after rendering is complete
		color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // not currently using stencil
		color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // not currently using stencil
		color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // don't care what type of texture the framebuffer was before rendering cuz it'll be cleared anyway
		color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // finally framebuffer layout should be presentable to screen

		VkAttachmentDescription depth_attachment_description = {};
		depth_attachment_description.flags = 0;
		depth_attachment_description.format = swap_chain->depth_image->format;
		depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// todo: consider making subpass creation more general. Not every render pass should operate the same.
		// some of them might want to use InputAttachments for general access of non-sampled image views.
		std::vector<VkAttachmentDescription> attachment_descriptions = { color_attachment_description, depth_attachment_description };

		// this handles the case for the implicit subpasses that occur for image layout transitions. this is to prevent the queue from accessing the framebuffer before its ready.
		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass = 0;
		subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpass_dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentReference color_attachment_reference = {};
		color_attachment_reference.attachment = 0; // framebuffer at index 0
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // use internal framebuffer as color texture

		VkAttachmentReference depth_attachment_reference = {};
		depth_attachment_reference.attachment = 1;
		depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.flags = 0;
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1; // only rendering to one single color buffer. can do more for deferred.
		subpass_description.pColorAttachments = &color_attachment_reference;
		subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.flags = 0;
		render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
		render_pass_create_info.pAttachments = attachment_descriptions.data(); // todo: you can add more here to perform deferred rendering.
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass_description;
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies = &subpass_dependency;

		VV_CHECK_SUCCESS(vkCreateRenderPass(device->logical_device, &render_pass_create_info, nullptr, &render_pass));
		has_been_created = true;
	}


	void VulkanRenderPass::shutDown()
	{
		if (render_pass != VK_NULL_HANDLE)
			vkDestroyRenderPass(_device->logical_device, render_pass, nullptr);
	}


	void VulkanRenderPass::beginRenderPass(VkCommandBuffer command_buffer, VkSubpassContents subpass_contents, VkFramebuffer framebuffer,
						 VkExtent2D extent, std::vector<VkClearValue> clear_values)
	{
		VkRenderPassBeginInfo render_pass_begin_info = {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = render_pass;
		render_pass_begin_info.framebuffer = framebuffer;
		render_pass_begin_info.renderArea.offset = { 0, 0 }; // define the size of render area
		render_pass_begin_info.renderArea.extent = extent;
		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, subpass_contents);
	}


	void VulkanRenderPass::endRenderPass(VkCommandBuffer command_buffer)
	{
		vkCmdEndRenderPass(command_buffer);
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////// Private
}