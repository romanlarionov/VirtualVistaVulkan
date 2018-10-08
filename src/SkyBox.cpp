
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
        m_device = device;
        m_radiance_descriptor_set = radiance_descriptor_set;
        m_environment_descriptor_set = environment_descriptor_set;
        m_mesh = mesh;
        m_radiance_map = radiance_map;
        m_diffuse_irradiance_map = diffuse_map;
        m_specular_irradiance_map = specular_map;
        m_brdf_lut = brdf_lut;
        m_max_mip_levels = specular_map->image->mip_levels;

        // SkyBox radiance descriptor set
    	m_radiance_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	m_radiance_image_info.imageView = radiance_map->image_view->image_view;
    	m_radiance_image_info.sampler = radiance_map->sampler->sampler;

        m_rad_write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_rad_write_set.pNext = NULL;
    	m_rad_write_set.dstSet = m_radiance_descriptor_set;
    	m_rad_write_set.dstBinding = 0;
    	m_rad_write_set.dstArrayElement = 0;
    	m_rad_write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	m_rad_write_set.descriptorCount = 1;
        m_rad_write_set.pImageInfo = &m_radiance_image_info;

        // Diffuse irradiance probe descriptor set
    	m_diffuse_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	m_diffuse_image_info.imageView = diffuse_map->image_view->image_view;
    	m_diffuse_image_info.sampler = diffuse_map->sampler->sampler;

        m_write_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_write_sets[0].pNext = NULL;
    	m_write_sets[0].dstSet = m_environment_descriptor_set;
    	m_write_sets[0].dstBinding = 0;
    	m_write_sets[0].dstArrayElement = 0;
    	m_write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	m_write_sets[0].descriptorCount = 1;
        m_write_sets[0].pImageInfo = &m_diffuse_image_info;

        // Specular irradiance probe descriptor set
        m_specular_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	m_specular_image_info.imageView = specular_map->image_view->image_view;
    	m_specular_image_info.sampler = specular_map->sampler->sampler;

        m_write_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_write_sets[1].pNext = NULL;
    	m_write_sets[1].dstSet = m_environment_descriptor_set;
    	m_write_sets[1].dstBinding = 1;
    	m_write_sets[1].dstArrayElement = 0;
    	m_write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	m_write_sets[1].descriptorCount = 1;
        m_write_sets[1].pImageInfo = &m_specular_image_info;

        // BRDF LUT descriptor set
        m_brdf_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    	m_brdf_image_info.imageView = brdf_lut->image_view->image_view;
    	m_brdf_image_info.sampler = brdf_lut->sampler->sampler;

        m_write_sets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_write_sets[2].pNext = NULL;
    	m_write_sets[2].dstSet = m_environment_descriptor_set;
    	m_write_sets[2].dstBinding = 2;
    	m_write_sets[2].dstArrayElement = 0;
    	m_write_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	m_write_sets[2].descriptorCount = 1;
        m_write_sets[2].pImageInfo = &m_brdf_image_info;
	}


	void SkyBox::shutDown()
	{
	}


    void SkyBox::updateDescriptorSet() const
    {
        vkUpdateDescriptorSets(m_device->logical_device, 1, &m_rad_write_set, 0, nullptr);
        vkUpdateDescriptorSets(m_device->logical_device, 3, m_write_sets.data(), 0, nullptr);
    }


    void SkyBox::bindSkyBoxDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const
    {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1,
                1, &m_radiance_descriptor_set, 0, nullptr);
    }


    void SkyBox::bindIBLDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const
    {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 2,
                1, &m_environment_descriptor_set, 0, nullptr);
    }


    void SkyBox::submitMipLevelPushConstants(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const
    {
        vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &m_max_mip_levels);
    }


    void SkyBox::render(VkCommandBuffer command_buffer)
    {
        m_mesh->bindBuffers(command_buffer);
        m_mesh->render(command_buffer);
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
