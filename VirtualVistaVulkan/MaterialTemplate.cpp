
#include "MaterialTemplate.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	MaterialTemplate::MaterialTemplate()
	{
	}


	MaterialTemplate::~MaterialTemplate()
	{
	}


    // Need access to raw vk pointers for creation.
    auto convertLayouts(std::vector<VulkanDescriptorSetLayout> layouts)
    {
        std::vector<VkDescriptorSetLayout> output;
        for (auto &l : layouts)
            output.push_back(l.layout);

        return output;
    }


    void MaterialTemplate::create(VulkanDevice *device, std::string name , Shader *shader, VkDescriptorPool descriptor_pool,
                                  std::vector<VulkanDescriptorSetLayout> descriptor_set_layouts,
                                  std::vector<DescriptorType> descriptor_orderings, VkSampler sampler, VulkanRenderPass *render_pass)
	{
		this->name = name;
		device_ = device;
        shader_ = shader;
		render_pass_ = render_pass;
        descriptor_pool_ = descriptor_pool;
        descriptor_set_layouts_ = descriptor_set_layouts; // only concerned here with material uniforms
        descriptor_orderings_ = descriptor_orderings;
        sampler_ = sampler;

		// initialize pipeline
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.flags = 0;
		pipeline_layout_create_info.setLayoutCount = (uint32_t)descriptor_set_layouts.size();
		pipeline_layout_create_info.pSetLayouts = convertLayouts(descriptor_set_layouts).data();
		pipeline_layout_create_info.pPushConstantRanges = nullptr; // todo: pass in as parameter
		pipeline_layout_create_info.pushConstantRangeCount = 0;

		VV_CHECK_SUCCESS(vkCreatePipelineLayout(device_->logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout));
		pipeline_.create(device, shader, pipeline_layout, render_pass, true, true); // todo: add option for settings passed.
	}

   
    void MaterialTemplate::shutDown()
	{
		if (pipeline_layout != VK_NULL_HANDLE)
			vkDestroyPipelineLayout(device_->logical_device, pipeline_layout, nullptr);

        pipeline_.shutDown();
        descriptor_set_layouts_[0].shutDown(); // todo: change
	}


	void MaterialTemplate::bindPipeline(VkCommandBuffer command_buffer, VkPipelineBindPoint bind_point)
	{
        pipeline_.bind(command_buffer, bind_point);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
