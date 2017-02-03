
#ifndef VIRTUALVISTA_MATERIALTEMPLATE_H
#define VIRTUALVISTA_MATERIALTEMPLATE_H

#include <vector>

#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "Shader.h"
//#include "Material.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSetLayout.h"

namespace vv
{
	class MaterialTemplate
	{
        friend class Material;

	public:
		std::string name;
		VkPipelineLayout pipeline_layout;

		MaterialTemplate();
		~MaterialTemplate();

		/*
		 * Fills in all resources needed for this template class including: pipeline, shaders, and render pass.
		 * This class serves as a model to create instances from. i.e. Material's are instances of MaterialTemplates
		 * note: trying to imitate THREE.js here.
         * todo: add push constants
		 */
		void create(VulkanDevice *device, std::string name , Shader *shader, VkDescriptorPool descriptor_pool,
                    std::vector<VulkanDescriptorSetLayout> descriptor_set_layouts,
                    std::vector<DescriptorType> descriptor_orderings, VkSampler sampler, VulkanRenderPass *render_pass);

		/*
		 * Removes all allocated sub parts that this class manages. Ensures no instances are present anymore.
		 */
		void shutDown();

		/*
		 * Access to pipeline binding is restricted through this public call.
		 */
        void bindPipeline(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point);
		
	private:
		VulkanDevice *device_;
		Shader *shader_;
		VulkanRenderPass *render_pass_;

		VulkanPipeline pipeline_;
        VkDescriptorPool descriptor_pool_;
		std::vector<VulkanDescriptorSetLayout> descriptor_set_layouts_;
        std::vector<DescriptorType> descriptor_orderings_;

        VkSampler sampler_;
		//std::vector<VulkanPushConstantRange> push_constant_ranges_;

	};
}

#endif // VIRTUALVISTA_MATERIALTEMPLATE_H