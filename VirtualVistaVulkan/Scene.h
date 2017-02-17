
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
	class Scene
	{
	public:
        std::unordered_map<std::string, MaterialTemplate *> material_templates;

		Scene();
		~Scene();

		/*
		 * Loads all resources and templates needed for model loading and descriptor set updating.
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
         * Requests that a model be loaded using the ModelManger and stored for rendering/updates.
         *
         * note: material_template will be applied to all submeshes for the model found.
         */
        Model* addModel(std::string path, std::string name, std::string material_template);

        /*
         *
         */
        void addCamera();

        /*
         * Updates the global scene descriptor sets with newly updates data.
         */
        void updateSceneUniforms(VkExtent2D extent);

        /*
         * Recursively renders each model.
         *
         * note: This will be automatically called within VulkanRenderer. There is no need in calling it from here.
         */
        void render(VkCommandBuffer command_buffer);

	private:
        VulkanDevice *_device                       = nullptr;
        VulkanRenderPass *_render_pass              = nullptr;
        ModelManager *_model_manager                = nullptr;
        bool _initialized                           = false;

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

        /*
         * Reads required shaders from file and creates all possible MaterialTemplates that can be used during execution.
         * These MaterialTemplates can be referenced by the name provided in the shader info file.
         */
        void createMaterialTemplates();

        /*
         *
         */
        void createPipelines();

        /*
         * Creates global descriptor pool from which all descriptor sets will be allocated from.
         */
        void createDescriptorPool();

        /*
         * Creates everything necessary for scene global uniforms.
         *
         * note: Each shader needs to accept these required uniform in its own descriptor set at set = 0.
         */
        void createSceneUniforms();

        /*
         * Creates single 16x anisotropic sampler used for all texture sampling.
         */
        void createSampler();

        /*
         * Utility function for creating descriptor set layouts.
         */
        void createVulkanDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout &layout);

        /*
         * Utility function for defining descriptor set bindings.
         */
        VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage) const;
	};
}

#endif // VIRTUALVISTA_SCENE_H