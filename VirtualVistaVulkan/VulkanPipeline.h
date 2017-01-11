
#ifndef VIRTUALVISTA_VULKANPIPELINE_H
#define VIRTUALVISTA_VULKANPIPELINE_H

#include <vector>

#include "Shader.h"
#include "VulkanSwapChain.h"

namespace vv
{
	struct VulkanPipeline
	{
	public:
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

		VulkanPipeline();
		~VulkanPipeline();

		/*
		 * 
		 */
		void create(VulkanDevice *device, Shader *shader, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts, VkRenderPass render_pass, bool depth_test_enable, bool depth_write_enable);

		/*
		 *
		 */
		void shutDown();

		/*
		 * 
		 */
		void bind(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point) const;
		
	private:
		VulkanDevice *device_;
	};
}

#endif // VIRTUALVISTA_VULKANPIPELINE_H