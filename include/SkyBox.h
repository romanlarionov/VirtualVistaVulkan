
#ifndef VIRTUALVISTA_SKYBOX_H
#define VIRTUALVISTA_SKYBOX_H

#include <array>
#include <vector>

#include "VulkanDevice.h"
#include "Mesh.h"
#include "TextureManager.h"

namespace vv
{
	class SkyBox
	{
	public:
		SkyBox();
		~SkyBox();

        /*
         * Creates a global skybox light probe that can be rendered during runtime as well as contribute to a scene's
         * lighting calculations by providing HDR diffuse + specular irradiance maps.
         */
        void create(VulkanDevice *device, VkDescriptorSet radiance_descriptor_set, VkDescriptorSet environment_descriptor_set,
                    Mesh *mesh, SampledTexture *radiance_map, SampledTexture *diffuse_map, SampledTexture *specular_map,
                    SampledTexture *brdf_lut);

        /*
		 * This is a special model that does not maintain ownership over any data.
		 */
		void shutDown();

        /*
         * When a skybox is set to a scene's "active_skybox", this function is used to update the skybox related
         * descriptor set data with the appropriate content.
         */
        void updateDescriptorSet() const;

        /*
         * The skybox sphere mesh only uses the radiance_map for rendering. This updates a special descriptor
         * set intended to be used exclusively by the "active_skybox".
         */
        void bindSkyBoxDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const;

        /*
         * Updates diffuse + specular irradiance maps as well as a preloaded BRDF LUT, which are used to 
         * calculate PBR image based lighting.
         */
        void bindIBLDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const;

        /*
         * Submits the maximum number of mip levels used for specular irradiance calculation.
         */
        void submitMipLevelPushConstants(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) const;

        /*
         * Calls the skybox sphere mesh's render function.
         */
        void render(VkCommandBuffer command_buffer);
	
	private:
		VulkanDevice *m_device;
        Mesh *m_mesh;
        uint32_t m_max_mip_levels;
	
        SampledTexture *m_brdf_lut;
        SampledTexture *m_radiance_map;
        SampledTexture *m_diffuse_irradiance_map;
        SampledTexture *m_specular_irradiance_map;

        VkDescriptorSet m_radiance_descriptor_set    = VK_NULL_HANDLE;
        VkDescriptorSet m_environment_descriptor_set = VK_NULL_HANDLE;

        VkWriteDescriptorSet m_rad_write_set;
        std::array<VkWriteDescriptorSet, 4> m_write_sets;

        VkDescriptorImageInfo m_radiance_image_info = {};
        VkDescriptorImageInfo m_diffuse_image_info  = {};
        VkDescriptorImageInfo m_specular_image_info = {};
        VkDescriptorImageInfo m_brdf_image_info     = {};

	};
}

#endif // VIRTUALVISTA_SKYBOX_H