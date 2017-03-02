
#ifndef VIRTUALVISTA_VULKANRENDERPASS_H
#define VIRTUALVISTA_VULKANRENDERPASS_H

#include <vector>

#include "VulkanSwapChain.h"
#include "VulkanDevice.h"

namespace vv
{
	class VulkanRenderPass
	{
	public:

		VkRenderPass render_pass;
		bool has_been_created; // todo: consider adding this to all classes. 

		VulkanRenderPass();
		~VulkanRenderPass();

		/*
		 *
		 */
		void create(VulkanDevice *device, VulkanSwapChain *swap_chain);

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

	private:
		VulkanDevice *device_;

	};
}

#endif // VIRTUALVISTA_VULKANRENDERPASS_H