
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
    struct SceneUniformBufferObject
    {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
        glm::vec4 camera_position;
	};

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
        void addLight();

        /*
         * Requests that a model be loaded using the ModelManger and stored for rendering/updates.
         *
         * note: material_template will be applied to all submeshes for the model found.
         */
        Model* addModel(std::string path, std::string name, std::string material_template);

        /*
         * Requests that a perspective camera be created.
         */
        Camera* addCamera(float fov_y, float near_plane, float far_plane);

        /*
         * Returns the currently marked "main" camera.
         */
        Camera* getActiveCamera() const;

        /*
         * Sets the specified camera as the active render viewpoint.
         */
        void setActiveCamera(Camera *camera);

        /*
         * Updates the global scene descriptor sets with newly updates data.
         */
        void updateSceneUniforms(VkExtent2D extent, float time);

        /*
         * Recursively renders each model.
         *
         * note: This will be automatically called within VulkanRenderer. There is no need in calling manually.
         */
        void render(VkCommandBuffer command_buffer);

	private:
        VulkanDevice *_device                       = nullptr;
        VulkanRenderPass *_render_pass              = nullptr;
        ModelManager *_model_manager                = nullptr;
        bool _initialized                           = false;

		VkSampler _sampler                          = VK_NULL_HANDLE;
		VkDescriptorPool _descriptor_pool           = VK_NULL_HANDLE;

        // General scene uniform
        VkDescriptorSetLayout _scene_descriptor_set_layout;
        VkDescriptorSet _scene_descriptor_set       = VK_NULL_HANDLE;
        SceneUniformBufferObject _scene_ubo;
		VulkanBuffer *_scene_uniform_buffer         = nullptr;

        // Lights uniform
        VkDescriptorSetLayout _lights_descriptor_set_layout;
        VkDescriptorSet _lights_descriptor_set      = VK_NULL_HANDLE;
        VulkanBuffer *_lights_uniform_buffer        = nullptr;

        // todo: think of better data structure. maybe something to help with culling
		std::vector<Light *> _lights;
		std::vector<Model *> _models;
		std::vector<Camera *> _cameras;

        Camera *_active_camera;

        /*
         * Reads required shaders from file and creates all possible MaterialTemplates that can be used during execution.
         * These MaterialTemplates can be referenced by the name provided in the shader info file.
         */
        void createMaterialTemplates();

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