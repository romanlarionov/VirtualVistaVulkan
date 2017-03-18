
#include "SkyBox.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	SkyBox::SkyBox()
	{
	}


	SkyBox::~SkyBox()
	{
	}


    void SkyBox::create(VulkanDevice *device, VkDescriptorSet radiance_descriptor_set, VkDescriptorSet environment_descriptor_set,
                        Mesh *mesh, SampledTexture *radiance_map, SampledTexture *diffuse_map, SampledTexture *specular_map,
                        SampledTexture *brdf_lut)
	{
        _device = device;
        _radiance_descriptor_set = radiance_descriptor_set;
        _environment_descriptor_set = environment_descriptor_set;
        _mesh = mesh;
        _radiance_map = radiance_map;
        _diffuse_irradiance_map = diffuse_map;
        _specular_irradiance_map = specular_map;
        _brdf_lut = brdf_lut;
        _max_mip_levels = specular_map->image->mip_levels;

        // SkyBox radiance descriptor set
    	_radiance_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	_radiance_image_info.imageView = radiance_map->image_view->image_view;
    	_radiance_image_info.sampler = radiance_map->sampler->sampler;

        _rad_write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	_rad_write_set.dstSet = _radiance_descriptor_set;
    	_rad_write_set.dstBinding = 0;
    	_rad_write_set.dstArrayElement = 0;
    	_rad_write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	_rad_write_set.descriptorCount = 1;
        _rad_write_set.pImageInfo = &_radiance_image_info;

        // Diffuse irradiance probe descriptor set
    	_diffuse_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	_diffuse_image_info.imageView = diffuse_map->image_view->image_view;
    	_diffuse_image_info.sampler = diffuse_map->sampler->sampler;

        _write_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	_write_sets[0].dstSet = _environment_descriptor_set;
    	_write_sets[0].dstBinding = 0;
    	_write_sets[0].dstArrayElement = 0;
    	_write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	_write_sets[0].descriptorCount = 1;
        _write_sets[0].pImageInfo = &_diffuse_image_info;

        // Specular irradiance probe descriptor set
        _specular_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	_specular_image_info.imageView = specular_map->image_view->image_view;
    	_specular_image_info.sampler = specular_map->sampler->sampler;

        _write_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	_write_sets[1].dstSet = _environment_descriptor_set;
    	_write_sets[1].dstBinding = 1;
    	_write_sets[1].dstArrayElement = 0;
    	_write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	_write_sets[1].descriptorCount = 1;
        _write_sets[1].pImageInfo = &_specular_image_info;

        // BRDF LUT descriptor set
        _brdf_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	_brdf_image_info.imageView = brdf_lut->image_view->image_view;
    	_brdf_image_info.sampler = brdf_lut->sampler->sampler;

        _write_sets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	_write_sets[2].dstSet = _environment_descriptor_set;
    	_write_sets[2].dstBinding = 2;
    	_write_sets[2].dstArrayElement = 0;
    	_write_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	_write_sets[2].descriptorCount = 1;
        _write_sets[2].pImageInfo = &_brdf_image_info;
	}


	void SkyBox::shutDown()
	{
	}


    void SkyBox::updateDescriptorSet() const
    {
        vkUpdateDescriptorSets(_device->logical_device, 1, &_rad_write_set, 0, nullptr);
        vkUpdateDescriptorSets(_device->logical_device, 3, _write_sets.data(), 0, nullptr);
    }


    void SkyBox::bindSkyBoxDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const
    {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1,
                1, &_radiance_descriptor_set, 0, nullptr);
    }


    void SkyBox::bindIBLDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const
    {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 2,
                1, &_environment_descriptor_set, 0, nullptr);
    }


    void SkyBox::submitMipLevelPushConstants(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const
    {
        vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &_max_mip_levels);
    }


    void SkyBox::render(VkCommandBuffer command_buffer)
    {
        _mesh->bindBuffers(command_buffer);
        _mesh->render(command_buffer);
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
