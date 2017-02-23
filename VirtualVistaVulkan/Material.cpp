
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
        _device = device;

        // certain material templates dont take descriptor sets
        if (material_template->material_descriptor_set_layout)
        {
            VkDescriptorSetAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = descriptor_pool;
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = &material_template->material_descriptor_set_layout;

            VV_CHECK_SUCCESS(vkAllocateDescriptorSets(device->logical_device, &alloc_info, &_descriptor_set));
        }
    }


	void Material::shutDown()
	{
        for (auto &ubo : _uniform_buffers)
        {
            ubo->buffer->shutDown();
            delete ubo->buffer;
            delete ubo;
        }

        for (auto &i : _image_store)
        {
            i->shutDown();
            delete i;
        }

        for (auto &tex : _textures)
        {
            tex->view->shutDown();
            delete tex->view;
            delete tex;
        }
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

        auto position = _uniform_buffers.size();
        _uniform_buffers.push_back(store);

        VkWriteDescriptorSet write_set = {};

        write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = _descriptor_set;
		write_set.dstBinding = binding;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_set.descriptorCount = 1; // how many elements to update
        write_set.pBufferInfo = &_uniform_buffers[position]->info;

        _write_sets.push_back(write_set);
    }


    void Material::addTexture(VulkanImage *texture, int binding, VkSampler sampler)
    {
        _image_store.push_back(texture);
        VulkanImageView *texture_image_view = new VulkanImageView();
        texture_image_view->create(_device, texture);

		VkDescriptorImageInfo image_info = {};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = texture_image_view->image_view;
		image_info.sampler = sampler;

        TextureStore *store = new TextureStore();
        store->info = image_info;
        store->view = texture_image_view;

        auto position = _textures.size();
        _textures.push_back(store);

        VkWriteDescriptorSet write_set = {};
        
        write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = _descriptor_set;
		write_set.dstBinding = binding; // location in layout
		write_set.dstArrayElement = 0; // if sending array of uniforms
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_set.descriptorCount = 1; // how many elements to update
        write_set.pImageInfo = &_textures[position]->info;

        _write_sets.push_back(write_set);
    }


    void Material::updateDescriptorSets() const
    {
		vkUpdateDescriptorSets(_device->logical_device, static_cast<uint32_t>(_write_sets.size()), _write_sets.data(), 0, nullptr);
    }


    void Material::bindDescriptorSets(VkCommandBuffer command_buffer) const
    {
        if (material_template->material_descriptor_set_layout)
        {
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material_template->pipeline_layout, 1,
                1, &_descriptor_set, 0, nullptr);
        }
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
