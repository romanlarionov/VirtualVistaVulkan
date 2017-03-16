
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
         * 
         */
        void create(VulkanDevice *device, VkDescriptorSet radiance_descriptor_set, VkDescriptorSet environment_descriptor_set,
                    Mesh *mesh, SampledTexture *radiance_map, SampledTexture *diffuse_map, SampledTexture *specular_map,
                    SampledTexture *brdf_lut);

        /*
		 * 
		 */
		void shutDown();

        /*
         *
         */
        void updateDescriptorSet();

        /*
         *
         */
        void bindSkyBoxDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);

        /*
         *
         */
        void bindIBLDescriptorSets(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);

        /*
         *
         */
        void render(VkCommandBuffer command_buffer);
	
	private:
		VulkanDevice *_device;
        Mesh *_mesh;
	
        SampledTexture *_brdf_lut;
        SampledTexture *_radiance_map;
        SampledTexture *_diffuse_irradiance_map;
        SampledTexture *_specular_irradiance_map;

        VkDescriptorSet _radiance_descriptor_set    = VK_NULL_HANDLE;
        VkDescriptorSet _environment_descriptor_set = VK_NULL_HANDLE;

        std::array<VkWriteDescriptorSet, 4> _write_sets;

        VkDescriptorImageInfo _radiance_image_info = {};
        VkDescriptorImageInfo _diffuse_image_info  = {};
        VkDescriptorImageInfo _specular_image_info = {};
        VkDescriptorImageInfo _brdf_image_info     = {};
	};
}

#endif // VIRTUALVISTA_SKYBOX_H