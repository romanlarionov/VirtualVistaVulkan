
#ifndef VIRTUALVISTA_SCENE_H
#define VIRTUALVISTA_SCENE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "VulkanDevice.h"
#include "SkyBox.h"
#include "VulkanRenderPass.h"
#include "VulkanSampler.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Model.h"
#include "Light.h"
#include "Model.h"
#include "Camera.h"

namespace vv
{
    struct LightData
    {
        glm::vec4 position;
        glm::vec4 irradiance;
    };

    struct LightUniformBufferObject
    {
        LightData lights[VV_MAX_LIGHTS];
    };

    struct SceneUniformBufferObject
    {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
        glm::mat4 normalMat;
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
         * Requests that a point light be created.
         */
        Light* addLight(glm::vec4 irradiance, float radius);

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
         * Adds a global skybox using a cube to render.
         */
        SkyBox* addSkyBox(std::string path, std::string radiance_map_name, std::string diffuse_map_name,
                          std::string specular_map_name, std::string brdf_lut_name);

        /*
         * Returns the currently marked "main" camera.
         */
        Camera* getActiveCamera() const;

        /*
         * Returns the currently marked active skybox.
         */
        SkyBox* getActiveSkyBox() const;

        /*
         * Sets the specified camera as the active render viewpoint.
         */
        void setActiveCamera(Camera *camera);

        /*
         * Specifies that the given skybox be rendered and used as environment lighting for scene.
         */
        void setActiveSkyBox(SkyBox *skybox);

        /*
         * Updates the global scene descriptor sets with newly updates data.
         */
        void updateUniformData(VkExtent2D extent, float time);

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
        TextureManager *_texture_manager            = nullptr;
        bool _initialized                           = false;

		VulkanSampler *_sampler                     = nullptr;
		VkDescriptorPool _descriptor_pool           = VK_NULL_HANDLE;

        // General scene uniform
        VkDescriptorSetLayout _scene_descriptor_set_layout;
        VkDescriptorSet _scene_descriptor_set       = VK_NULL_HANDLE;
        SceneUniformBufferObject _scene_ubo;
		VulkanBuffer *_scene_uniform_buffer         = nullptr;

        // Light uniforms
        LightUniformBufferObject _lights_ubo;
        VkDescriptorSetLayout _lights_descriptor_set_layout;
		VulkanBuffer *_lights_uniform_buffer         = nullptr;

        VkDescriptorSetLayout _environment_descriptor_set_layout;
        VkDescriptorSetLayout _radiance_descriptor_set_layout;
        VkDescriptorSet _environment_descriptor_set  = VK_NULL_HANDLE; // used for IBL calculations
        VkDescriptorSet _radiance_descriptor_set     = VK_NULL_HANDLE; // applied to skybox model

        // todo: think of better data structure. maybe something to help with culling
		std::vector<Light *> _lights;
		std::vector<Model *> _models;
		std::vector<Camera *> _cameras;
		std::vector<SkyBox *> _skyboxes;

        Camera *_active_camera;
        SkyBox *_active_skybox;
        bool _has_active_camera;
        bool _has_active_skybox;

        /*
         * Reads required shaders from file and creates all possible MaterialTemplates that can be used during execution.
         * These MaterialTemplates can be referenced by the name provided in the shader info file.
         */
        void createMaterialTemplates();

        /*
         * Creates global descriptor pool from which all descriptor sets will be allocated from.
         */
        void createDescriptorPool();


        void createSceneUniforms();

        /*
         * Creates everything necessary for scene global uniforms.
         */
        void createEnvironmentUniforms();

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