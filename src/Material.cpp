
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


    void Material::create(VulkanDevice *device, MaterialTemplate *material_template, VkDescriptorPool descriptor_pool)
    {
        this->material_template = material_template;
        m_device = device;

        // certain material templates don't take descriptor sets
        if (material_template->material_descriptor_set_layout)
        {
            VkDescriptorSetAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = descriptor_pool;
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = &material_template->material_descriptor_set_layout;

            VV_CHECK_SUCCESS(vkAllocateDescriptorSets(device->logical_device, &alloc_info, &m_descriptor_set));
        }
    }


    void Material::shutDown()
    {
        for (auto &ubo : m_uniform_buffers)
        {
            ubo->buffer->shutDown();
            delete ubo->buffer;
            delete ubo;
        }

        m_uniform_buffers.clear();
        m_textures.clear();
        m_write_sets.clear();
    }


    void Material::addUniformBuffer(VulkanBuffer *uniform_buffer, int binding)
    {
        VkDescriptorBufferInfo buffer_info = {};
    	buffer_info.buffer = uniform_buffer->buffer;
    	buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;

        UBOStore *store = new UBOStore;
        store->info = buffer_info;
        store->buffer = uniform_buffer;

        auto position = m_uniform_buffers.size();
        m_uniform_buffers.push_back(store);

        VkWriteDescriptorSet write_set = {};

        write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	write_set.dstSet = m_descriptor_set;
    	write_set.dstBinding = binding;
    	write_set.dstArrayElement = 0;
    	write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	write_set.descriptorCount = 1; // how many elements to update
        write_set.pBufferInfo = &m_uniform_buffers[position]->info;

        m_write_sets.push_back(write_set);
    }


    void Material::addTexture(SampledTexture *texture, int binding)
    {
        VkDescriptorImageInfo image_info = {};
    	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	image_info.imageView   = texture->image_view->image_view;
    	image_info.sampler     = texture->sampler->sampler;

        TextureStore *store = new TextureStore();
        store->info = image_info;
        store->texture = texture;

        auto position = m_textures.size();
        m_textures.push_back(store);

        VkWriteDescriptorSet write_set = {};
        write_set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	write_set.dstSet          = m_descriptor_set;
    	write_set.dstBinding      = binding;
    	write_set.dstArrayElement = 0;
    	write_set.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	write_set.descriptorCount = 1;
        write_set.pImageInfo      = &m_textures[position]->info;

        m_write_sets.push_back(write_set);
    }


    void Material::updateDescriptorSets() const
    {
    	vkUpdateDescriptorSets(m_device->logical_device, static_cast<uint32_t>(m_write_sets.size()), m_write_sets.data(), 0, nullptr);
    }


    void Material::bindDescriptorSets(VkCommandBuffer command_buffer, VkPipelineBindPoint pipeline_bind_point) const
    {
        if (material_template->material_descriptor_set_layout)
        {
            vkCmdBindDescriptorSets(command_buffer, pipeline_bind_point, material_template->pipeline_layout, 1,
                1, &m_descriptor_set, 0, nullptr);
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////////////// Private
}
