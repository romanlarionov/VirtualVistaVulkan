
#include "Material.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Material::Material()
	{
	}


	Material::~Material()
	{
	}

	
    void Material::create(MaterialTemplate *material_template)
    {
        this->material_template = material_template;

        VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = material_template->descriptor_pool_;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &material_template->descriptor_set_layouts_[0].layout; // todo: change

		VV_CHECK_SUCCESS(vkAllocateDescriptorSets(material_template->device_->logical_device, &alloc_info, &descriptor_set_));
    }


	void Material::shutDown()
	{
        for (auto &ubo : uniform_buffers_)
            ubo->shutDown();

        for (auto &tex : textures_)
            tex->shutDown();
	}


    void Material::addUniformBuffer(VulkanBuffer *uniform_buffer, int binding)
    {
        VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = uniform_buffer->buffer;
		buffer_info.offset = 0;
		buffer_info.range = uniform_buffer->range;

        VkWriteDescriptorSet write_set = {};

        write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = descriptor_set_;
		write_set.dstBinding = binding;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_set.descriptorCount = 1; // how many elements to update
		write_set.pBufferInfo = &buffer_info;

        write_sets_.push_back(write_set);
    }


    void Material::addTexture(VulkanImage *texture, int binding)
    {
        VulkanImageView *texture_image_view = new VulkanImageView();
        texture_image_view->create(material_template->device_, texture);
        textures_.push_back(texture_image_view);

		VkDescriptorImageInfo image_info = {};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = texture_image_view->image_view;
		image_info.sampler = material_template->sampler_;

        VkWriteDescriptorSet write_set = {};
        
        write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = descriptor_set_;
		write_set.dstBinding = binding; // location in layout
		write_set.dstArrayElement = 0; // if sending array of uniforms
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_set.descriptorCount = 1; // how many elements to update
		write_set.pImageInfo = &image_info;

        write_sets_.push_back(write_set);
    }


    void Material::updateDescriptorSets() const
    {
        // todo: find out if doing this per material is good idea. might want to have batched update
		vkUpdateDescriptorSets(material_template-> device_->logical_device, (uint32_t)write_sets_.size(), write_sets_.data(), 0, nullptr);
    }


    void Material::bindDescriptorSets(VkCommandBuffer command_buffer, VkDescriptorSet general_descriptor_set) const
    {
        std::array<VkDescriptorSet, 2> descriptor_sets = { descriptor_set_, general_descriptor_set };
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material_template->pipeline_layout, 0,
                                (uint32_t)descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);
    }


    std::vector<DescriptorType> Material::getDescriptorOrdering() const
    {
        return material_template->descriptor_orderings_;
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
