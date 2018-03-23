
#include "VulkanRenderPass.h"

namespace vv
{
    void VulkanRenderPass::addAttachment(VkFormat format,
                                         VkSampleCountFlagBits sample_count,
                                         VkAttachmentLoadOp load_op,
                                         VkAttachmentStoreOp store_op,
                                         VkAttachmentLoadOp stencil_load_op,
                                         VkAttachmentStoreOp stencil_store_op,
                                         VkImageLayout input_layout,
                                         VkImageLayout output_layout)
    {
        VkAttachmentDescription attachment_description = {};
        attachment_description.flags          = 0;
        attachment_description.format         = format;
        attachment_description.samples        = sample_count;
        attachment_description.loadOp         = load_op;
        attachment_description.storeOp        = store_op;
        attachment_description.stencilLoadOp  = stencil_load_op;
        attachment_description.stencilStoreOp = stencil_store_op;
        attachment_description.initialLayout  = input_layout;
        attachment_description.finalLayout    = output_layout;

        _attachment_descriptions.push_back(attachment_description);
    }


    void VulkanRenderPass::create(VulkanDevice *device, VkPipelineBindPoint bind_point)
    {
        VV_ASSERT(device != nullptr, "Vulkan Device is NULL");
        _device = device;

        std::vector<VkAttachmentReference> color_references;
        VkAttachmentReference depth_reference;
        bool has_color = false;
        bool has_depth = false;
        uint32_t attachment_idx = 0;

        for (auto &attach : _attachment_descriptions)
        {
            // note: if i need to attach any fancy stuff, i can always add additional
            //       conditions here and differentiate by setting the layout parameter

            if (isDepthStencil(attach))
            {
                VV_ASSERT(!has_depth, "Trying to attach multiple depth buffers to same subpass");
                depth_reference.attachment = attachment_idx;
                depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                has_depth = true;
            }
            else
            {
                color_references.push_back({attachment_idx, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                has_color = true;
            }
            attachment_idx++;
        }

        // this handles the case for the implicit subpasses that occur for image layout transitions.
        // i.e. this is to prevent the command queue from accessing the framebuffer before its ready.
        std::array<VkSubpassDependency, 2> subpass_dependencies;
        subpass_dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
        subpass_dependencies[0].dstSubpass      = 0;
        subpass_dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpass_dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        subpass_dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpass_dependencies[1].srcSubpass      = 0;
        subpass_dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        subpass_dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpass_dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        subpass_dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDescription subpass_description = {};
        subpass_description.flags                = 0;
        subpass_description.pipelineBindPoint    = bind_point;
        if (has_color)
        {
            subpass_description.colorAttachmentCount = (uint32_t)color_references.size();
            subpass_description.pColorAttachments    = color_references.data();
        }
        if (has_depth)
            subpass_description.pDepthStencilAttachment = &depth_reference;

        VkRenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.flags           = 0;
        render_pass_create_info.attachmentCount = (uint32_t)_attachment_descriptions.size();
        render_pass_create_info.pAttachments    = _attachment_descriptions.data();
        render_pass_create_info.subpassCount    = 1;
        render_pass_create_info.pSubpasses      = &subpass_description;
        render_pass_create_info.dependencyCount = (uint32_t)subpass_dependencies.size();
        render_pass_create_info.pDependencies   = subpass_dependencies.data();

        VV_CHECK_SUCCESS(vkCreateRenderPass(device->logical_device, &render_pass_create_info, nullptr, &render_pass));
    }


    void VulkanRenderPass::shutDown()
    {
        if (render_pass != VK_NULL_HANDLE)
            vkDestroyRenderPass(_device->logical_device, render_pass, nullptr);

        _attachment_descriptions.clear();
    }


    VkFramebuffer VulkanRenderPass::createFramebuffer(std::vector<VkImageView> &attachments, VkExtent2D extent) const
    {
        VV_ASSERT(!attachments.empty(), "Attempting to create VkFramebuffer object with no attachments");

        VkFramebufferCreateInfo frame_buffer_create_info = {};
        frame_buffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_create_info.flags           = 0;
        frame_buffer_create_info.renderPass      = render_pass;
        frame_buffer_create_info.attachmentCount = (uint32_t)attachments.size();
        frame_buffer_create_info.pAttachments    = attachments.data();
        frame_buffer_create_info.width           = extent.width;
        frame_buffer_create_info.height          = extent.height;
        frame_buffer_create_info.layers          = 1;

        VkFramebuffer frame_buffer;
        VV_CHECK_SUCCESS(vkCreateFramebuffer(_device->logical_device, &frame_buffer_create_info, nullptr, &frame_buffer));
        return frame_buffer;
    }


    void VulkanRenderPass::beginRenderPass(VkCommandBuffer command_buffer,
                                           VkSubpassContents subpass_contents,
                                           VkFramebuffer framebuffer,
    					                   VkExtent2D extent,
                                           std::vector<VkClearValue> clear_values)
    {
        VkRenderPassBeginInfo render_pass_begin_info = {};
        render_pass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass        = render_pass;
        render_pass_begin_info.framebuffer       = framebuffer;
        render_pass_begin_info.renderArea.offset = { 0, 0 }; // define the size of render area
        render_pass_begin_info.renderArea.extent = extent;
        render_pass_begin_info.clearValueCount   = (uint32_t)clear_values.size();
        render_pass_begin_info.pClearValues      = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, subpass_contents);
    }


    void VulkanRenderPass::endRenderPass(VkCommandBuffer command_buffer)
    {
        vkCmdEndRenderPass(command_buffer);
    }
}