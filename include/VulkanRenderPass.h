
#ifndef VIRTUALVISTA_VULKANRENDERPASS_H
#define VIRTUALVISTA_VULKANRENDERPASS_H

#include <vector>
#include <algorithm>

#include "VulkanDevice.h"

namespace vv
{
	class VulkanRenderPass
	{
	public:
		VkRenderPass render_pass;

		VulkanRenderPass() = default;
		~VulkanRenderPass() = default;

		/*
		 * 
		 */
		void create(VulkanDevice *device, VkPipelineBindPoint bind_point);

		/*
		 *
		 */
		void shutDown();

		/*
		 * Informs Vulkan to start using this render pass object for drawing.
		 */
		void beginRenderPass(VkCommandBuffer command_buffer, VkSubpassContents subpass_contents, VkFramebuffer framebuffer,
							 VkExtent2D extent, std::vector<VkClearValue> clear_values);

		/*
		 * Tells Vulkan that this render pass has been successfully used for rendering and should quit.
		 */
		void endRenderPass(VkCommandBuffer command_buffer);

        /*
		 * 
		 */
        void addAttachment(VkFormat format,
                           VkSampleCountFlagBits sample_count,
                           VkAttachmentLoadOp load_op,
                           VkAttachmentStoreOp store_op,
                           VkAttachmentLoadOp stencil_load_op,
                           VkAttachmentStoreOp stencil_store_op,
                           VkImageLayout input_layout,
                           VkImageLayout output_layout);

        /*
         *
         */
        void addSubpassDependency(uint32_t src_subpass,
                                  uint32_t dst_subpass,
                                  VkPipelineStageFlags src_pipeline_stage,
                                  VkPipelineStageFlags dst_pipeline_stage,
                                  VkAccessFlags src_access_flags,
                                  VkAccessFlags dst_access_flags);

        /*
         *
         */
        VkFramebuffer createFramebuffer(std::vector<VkImageView> &attachments, VkExtent2D extent) const;

	private:
		VulkanDevice *_device;
        VkPipelineBindPoint _bind_point;
		std::vector<VkAttachmentDescription> _attachment_descriptions;

        bool hasDepth(VkAttachmentDescription description)
        {
            std::vector<VkFormat> formats =
            {
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            };
            return std::find(formats.begin(), formats.end(), description.format) != std::end(formats);
        }

        bool hasStencil(VkAttachmentDescription description)
        {
            std::vector<VkFormat> formats =
            {
                VK_FORMAT_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            };
            return std::find(formats.begin(), formats.end(), description.format) != std::end(formats);
        }

        bool isDepthStencil(VkAttachmentDescription description)
        {
            return(hasDepth(description) || hasStencil(description));
        }
	};
}

#endif // VIRTUALVISTA_VULKANRENDERPASS_H