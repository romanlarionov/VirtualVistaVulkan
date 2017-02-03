
#ifndef VIRTUALVISTA_VULKANPIPELINE_H
#define VIRTUALVISTA_VULKANPIPELINE_H

#include <vector>

#include "Shader.h"
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
		 * Creates a pipeline abstraction.
         * note: this is single use pipeline for now. No current way to alter the pipeline
         *       created outside of shader modules, descriptor set layouts, and push constants.
		 */
		void create(VulkanDevice *device, Shader *shader, VkPipelineLayout pipeline_layout,
                    VulkanRenderPass *render_pass, bool depth_test_enable, bool depth_write_enable);

		/*
		 *
		 */
		void shutDown();

		/*
		 * Activates this pipeline for use during command buffer recording.
		 */
		void bind(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point) const;

        // todo: add functions that add single attachment states
		
	private:
		VulkanDevice *device_;
	};
}

#endif // VIRTUALVISTA_VULKANPIPELINE_H