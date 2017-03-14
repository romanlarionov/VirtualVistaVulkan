
#include "Scene.h"

#include <string>
#include <fstream>
#include <chrono>

#include "Settings.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Scene::Scene()
	{
	}


	Scene::~Scene()
	{
	}


	void Scene::create(VulkanDevice *device, VulkanRenderPass *render_pass)
	{
        _device = device;
        _render_pass = render_pass;

        createDescriptorPool();
        createSceneUniforms();
        createMaterialTemplates(); // Load material templates to prepare for model loading queries

        _texture_manager = new TextureManager();
        _texture_manager->create(_device);

        _model_manager = new ModelManager();
        _model_manager->create(_device, _texture_manager, _descriptor_pool);

        _initialized = true;
	}


	void Scene::shutDown()
	{
        for (auto &t : material_templates)
        {
            vkDestroyDescriptorSetLayout(_device->logical_device, t.second->material_descriptor_set_layout, nullptr);
            if (t.second->non_standard_descriptor_set_layout)
                vkDestroyDescriptorSetLayout(_device->logical_device, t.second->material_descriptor_set_layout, nullptr);
            t.second->shader->shutDown(); delete t.second->shader;
            
            vkDestroyPipelineLayout(_device->logical_device, t.second->pipeline_layout, nullptr);
            t.second->pipeline->shutDown(); delete t.second->pipeline;

            delete t.second;
        }

        for (auto &l : _lights)
        {
            l->shutDown();
            delete l;
        }

        for (auto &m : _models)
        {
            m->shutDown();
            delete m;
        }

        for (auto &c : _cameras)
        {
            c->shutDown();
            delete c;
        }

        _scene_uniform_buffer->shutDown(); delete _scene_uniform_buffer;
        _lights_uniform_buffer->shutDown(); delete _lights_uniform_buffer;
        vkDestroyDescriptorSetLayout(_device->logical_device, _scene_descriptor_set_layout, nullptr);

	  	vkDestroyDescriptorPool(_device->logical_device, _descriptor_pool, nullptr);
        _texture_manager->shutDown(); delete _texture_manager;
        _model_manager->shutDown(); delete _model_manager;
   }


    Light* Scene::addLight(glm::vec4 irradiance, float radius)
    {
        VV_ASSERT(_lights.size() < VV_MAX_LIGHTS,
            "ERROR: number of lights exceeds VV_MAX_LIGHTS. Either remove allocated light or increase settings cap.");
        Light *light = new Light();
        light->create(irradiance, radius);
        _lights.push_back(light);
        return light;
    }


    Model* Scene::addModel(std::string path, std::string name, std::string material_template)
    {
        VV_ASSERT(_initialized, "ERROR: scene needs to be initialized before adding models");
        Model *model = new Model();
        _model_manager->loadModel(path, name, material_templates[material_template], model);
        _models.push_back(model);
        return model;
    }


    Camera* Scene::addCamera(float fov_y, float near_plane, float far_plane)
    {
        VV_ASSERT(_initialized, "ERROR: scene needs to be initialized before adding cameras");
        Camera *camera = new Camera();
        camera->create(fov_y, near_plane, far_plane);
        _cameras.push_back(camera);
        return camera;
    }


    Camera* Scene::getActiveCamera() const
    {
        return _active_camera;
    }


    void Scene::setActiveCamera(Camera *camera)
    {
        _active_camera = camera;
    }


    void Scene::updateSceneUniforms(VkExtent2D extent, float delta_time)
    {
        VV_ASSERT(_active_camera != nullptr, "ERROR: main camera has not been initialized");

        for (auto i = 0; i < _lights.size(); ++i)
        { 
            _lights_ubo.lights[i].position = glm::vec4(_lights[i]->getPosition(), 0.0f);
            _lights_ubo.lights[i].irradiance = _lights[i]->irradiance;
            //_lights_ubo.lights[i].radius = _lights[i]->radius;
        }
        _lights_uniform_buffer->updateAndTransfer(&_lights_ubo);

        _scene_ubo.view = _active_camera->getViewMatrix();
        _scene_ubo.projection = _active_camera->getProjectionMatrix(extent.width / static_cast<float>(extent.height));
        _scene_ubo.camera_position = glm::vec4(_active_camera->getPosition(), 1.0);
        _scene_uniform_buffer->updateAndTransfer(&_scene_ubo);
    }


    void Scene::render(VkCommandBuffer command_buffer)
    {
        bool first_run = true;
        MaterialTemplate *curr_template = nullptr;
        
        for (auto &model : _models)
        {
            // reduce pipeline state switches as much as possible
            if (first_run || (curr_template->name != model->material_template->name))
            {
                first_run = false;
                curr_template = model->material_template;
                curr_template->pipeline->bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

                // bind updated scene descriptor set
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, curr_template->pipeline_layout, 0, 1, &_scene_descriptor_set, 0, nullptr);
            }

            std::vector<Mesh *> meshes = _model_manager->_loaded_meshes[model->_data_handle];
            std::vector<Material *> materials = _model_manager->_loaded_materials[model->_data_handle][model->_material_id_set];

            // Update scene descriptor sets
            _scene_ubo.model = model->getModelMatrix();
            _scene_ubo.normalMat = glm::transpose(glm::inverse(model->getModelMatrix()));

            // Render all submeshes within this model
            for (auto &mesh : meshes)
            {
                Material *material = materials[mesh->material_id];
                material->bindDescriptorSets(command_buffer);
                mesh->bindBuffers(command_buffer);
                mesh->render(command_buffer);
            }
        }
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
    void Scene::createMaterialTemplates()
    {
        std::string shader_file = Settings::inst()->getShaderDirectory() + "shader_info.txt";
        std::ifstream file(shader_file);

        VV_ASSERT(file.is_open(), "Failed to open shader_info.txt. Was it moved or renamed?");

        // loop through required shaders in info file and initialize them.
        std::string curr_shader_name;
        while (std::getline(file, curr_shader_name))
        {
            MaterialTemplate *material_template = new MaterialTemplate();
            material_template->name = curr_shader_name; // note: apply name to template based on name assigned to spriv shader

            // Shader
            Shader *shader = new Shader();
            shader->create(_device, material_template->name);
            material_template->shader = shader;

            // Descriptor Set Layouts
            std::vector<VkDescriptorSetLayout> descriptor_set_layouts;

            /// Scene descriptor set layout
            descriptor_set_layouts.push_back(_scene_descriptor_set_layout);

            /// standard material descriptor layout
            std::vector<VkDescriptorSetLayoutBinding> temp_bindings_buffer;
            for (auto &o : shader->standard_material_descriptor_orderings)
                temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(o.binding, o.type, 1, o.shader_stage));

            createVulkanDescriptorSetLayout(_device->logical_device, temp_bindings_buffer, material_template->material_descriptor_set_layout);
            descriptor_set_layouts.push_back(material_template->material_descriptor_set_layout);

            /// non-standard descriptor set layout
            if (!shader->non_standard_descriptor_orderings.empty())
            {
                temp_bindings_buffer.clear();
                for (auto &o : shader->non_standard_descriptor_orderings)
                    temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(o.binding, o.type, 1, o.shader_stage));

                createVulkanDescriptorSetLayout(_device->logical_device, temp_bindings_buffer, material_template->non_standard_descriptor_set_layout);
                descriptor_set_layouts.push_back(material_template->non_standard_descriptor_set_layout);
            }

            // Pipeline
            VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		    pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
		    pipeline_layout_create_info.pPushConstantRanges = nullptr; // todo: add push constants
		    pipeline_layout_create_info.pushConstantRangeCount = 0;

		    VV_CHECK_SUCCESS(vkCreatePipelineLayout(_device->logical_device, &pipeline_layout_create_info, nullptr, &material_template->pipeline_layout));

            VulkanPipeline *pipeline = new VulkanPipeline();

            // todo: all pipelines should be created via single call to vkCreateGraphicsPipelines
		    pipeline->create(_device, material_template->shader, material_template->pipeline_layout, _render_pass, true, true); // todo: add option for settings passed.
            material_template->pipeline = pipeline;

            material_templates[material_template->name] = material_template;
        }

        // todo: bind all created pipelines at once to be more efficient.

        file.close();
    }


    void Scene::createDescriptorPool()
	{
        std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = Settings::inst()->getMaxUniformBuffers();
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = Settings::inst()->getMaxCombinedImageSamplers();

		VkDescriptorPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		create_info.pPoolSizes = pool_sizes.data();
		create_info.maxSets = Settings::inst()->getMaxDescriptorSets();
		create_info.flags = 0; // can be: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT

        // set up global descriptor pool
		VV_CHECK_SUCCESS(vkCreateDescriptorPool(_device->logical_device, &create_info, nullptr, &_descriptor_pool));
	}

    
    void Scene::createSceneUniforms()
	{
        // MVP matrix data
        _scene_ubo = { glm::mat4(), glm::mat4(), glm::mat4(), glm::mat4(), glm::vec4() };
        _scene_uniform_buffer = new VulkanBuffer();
        _scene_uniform_buffer->create(_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(_scene_ubo));

        // Lights data
        for (auto i = 0; i < VV_MAX_LIGHTS; ++i)
        {
            _lights_ubo.lights[i].position = glm::vec4();
            _lights_ubo.lights[i].irradiance = glm::vec4();
            //_lights_ubo.lights[i].radius = 0.0f;
        }

        _lights_uniform_buffer = new VulkanBuffer();
        _lights_uniform_buffer->create(_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(_lights_ubo));

        /// Layout
        std::vector<VkDescriptorSetLayoutBinding> temp_bindings_buffer;
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT));
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
        createVulkanDescriptorSetLayout(_device->logical_device, temp_bindings_buffer, _scene_descriptor_set_layout);

        /// Descriptor Set
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _descriptor_pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &_scene_descriptor_set_layout;

		VV_CHECK_SUCCESS(vkAllocateDescriptorSets(_device->logical_device, &alloc_info, &_scene_descriptor_set));

        std::array<VkWriteDescriptorSet, 2> write_sets;

		VkDescriptorBufferInfo scene_buffer_info = {};
		scene_buffer_info.buffer = _scene_uniform_buffer->buffer;
		scene_buffer_info.offset = 0;
		scene_buffer_info.range = sizeof(_scene_ubo);

		write_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_sets[0].dstSet = _scene_descriptor_set;
		write_sets[0].dstBinding = 0;
		write_sets[0].dstArrayElement = 0;
		write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[0].descriptorCount = 1; // how many elements to update
		write_sets[0].pBufferInfo = &scene_buffer_info;

        VkDescriptorBufferInfo lights_buffer_info = {};
		lights_buffer_info.buffer = _lights_uniform_buffer->buffer;
		lights_buffer_info.offset = 0;
        lights_buffer_info.range = sizeof(_lights_ubo);

		write_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_sets[1].dstSet = _scene_descriptor_set;
		write_sets[1].dstBinding = 1;
		write_sets[1].dstArrayElement = 0;
		write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[1].descriptorCount = 1; // how many elements to update
		write_sets[1].pBufferInfo = &lights_buffer_info;
		
        vkUpdateDescriptorSets(_device->logical_device, 2, write_sets.data(), 0, nullptr);
	}


	/*void Scene::createSampler()
	{
		VkSamplerCreateInfo sampler_create_info = {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter = VK_FILTER_LINEAR; // VK_FILTER_NEAREST
		sampler_create_info.minFilter = VK_FILTER_LINEAR; // VK_FILTER_NEAREST

		// can be a number of things: https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkSamplerAddressMode
		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		sampler_create_info.anisotropyEnable = VK_TRUE;
		sampler_create_info.maxAnisotropy = 16; // max value

		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; // black, white, transparent
		sampler_create_info.unnormalizedCoordinates = VK_FALSE; // [0,1]
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f; // todo: figure out how lod works with these things

		VV_CHECK_SUCCESS(vkCreateSampler(_device->logical_device, &sampler_create_info, nullptr, &_sampler));
	}*/


    void Scene::createVulkanDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout &layout)
    {
        VkDescriptorSetLayoutCreateInfo layout_create_info = {};
	    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	    layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
	    layout_create_info.pBindings = bindings.data();

	    VV_CHECK_SUCCESS(vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &layout));
    }


    VkDescriptorSetLayoutBinding Scene::createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage) const
    {
        VkDescriptorSetLayoutBinding layout_binding = {};
        layout_binding.binding = binding;
		layout_binding.descriptorType = descriptor_type;
		layout_binding.descriptorCount = count; // number of these elements in array sent to device

		layout_binding.stageFlags = shader_stage;
		layout_binding.pImmutableSamplers = nullptr; // todo: figure out if I ever need this
        return layout_binding;
    }
}
