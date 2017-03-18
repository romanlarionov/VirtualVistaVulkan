
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

		VulkanRenderPass();
		~VulkanRenderPass();

		/*
		 * Creates a single use case render pass to be used with a standard forward rendering design.
         * No additional or dynamic subpass are currently available to be created.
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
		VulkanDevice *_device;
		bool _initialized;

	};
}

#endif // VIRTUALVISTA_VULKANRENDERPASS_H