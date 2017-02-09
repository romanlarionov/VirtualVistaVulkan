
#ifndef VIRTUALVISTA_SCENE_H
#define VIRTUALVISTA_SCENE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "ModelManager.h"
#include "Model.h"
#include "Light.h"
#include "Model.h"
#include "Camera.h"

namespace vv
{
	struct Scene
	{
	public:
        std::unordered_map<std::string, MaterialTemplate *> material_templates;

		Scene();
		~Scene();

		/*
		 * 
		 */
		void create(VulkanDevice *device, VulkanRenderPass *render_pass);

		/*
		 *
		 */
		void shutDown();

        /*
         * 
         */
        void signalAllLightsAdded();

        /*
        *
        */
        void addLight();

        /*
         *
         */
        Model* addModel(std::string path, std::string name, MaterialTemplate *material_template);

        /*
         *
         */
        void addCamera();

        /*
         * 
         */
        void render(VkCommandBuffer command_buffer);

	private:
        VulkanDevice *_device                       = nullptr;
        VulkanRenderPass *_render_pass              = nullptr;
        ModelManager *_model_manager                = nullptr;

		VkSampler _sampler                          = VK_NULL_HANDLE;

        // global setting data for shaders
        const uint32_t MAX_DESCRIPTOR_SETS          = 100;
        const uint32_t MAX_UNIFORM_BUFFERS          = 100;
        const uint32_t MAX_COMBINED_IMAGE_SAMPLERS  = 100;
		VkDescriptorPool _descriptor_pool           = VK_NULL_HANDLE;

        // General scene uniform
        VkDescriptorSetLayout _scene_descriptor_set_layout;
        VkDescriptorSet _scene_descriptor_set       = VK_NULL_HANDLE;
        UniformBufferObject _ubo;
		VulkanBuffer *_scene_uniform_buffer         = nullptr;

        // Lights uniform
        VkDescriptorSetLayout _lights_descriptor_set_layout;
        VkDescriptorSet _lights_descriptor_set      = VK_NULL_HANDLE;
        VulkanBuffer *_lights_uniform_buffer        = nullptr;

        //std::unordered_map<Handle, Light*> lights_;
		std::vector<Model*> _models; // todo: think of better data structure. maybe something to help with culling
		//std::unordered_map<Handle, Camera*> cameras_;

        void createMaterialTemplates();

        void createPipelines();

        void createDescriptorPool();

        void createSceneUniforms();

        void createSampler();

        VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage) const;
	};
}

#endif // VIRTUALVISTA_SCENE_H