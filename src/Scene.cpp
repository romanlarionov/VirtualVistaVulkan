
#include "Scene.h"

#include <string>
#include <fstream>
#include <chrono>

#include "Settings.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace vv
{
    void Scene::create(VulkanDevice *device, VulkanRenderPass *render_pass)
    {
        m_device = device;
        m_render_pass = render_pass;

        createDescriptorPool();
        createSceneDescriptorSetLayout();
        createEnvironmentUniforms();
        createMaterialTemplates(); // Load material templates to prepare for model loading queries

        m_texture_manager = new TextureManager();
        m_texture_manager->create(m_device);

        m_model_manager = new ModelManager();
        m_model_manager->create(m_device, m_texture_manager, m_descriptor_pool);

        m_initialized = true;
    }


    void Scene::shutDown()
    {
        for (auto &temp: material_templates)
        {
            // todo: this is a hack to work around some issue with the "dummy" material descriptor set
            //       layout not wanting to be destroyed... i'm still not sure what's wrong, but I get
            //       the feeling that it has something to do with me never actually using it to render any models
            if (temp.second.name != "dummy")
                vkDestroyDescriptorSetLayout(m_device->logical_device, temp.second.material_descriptor_set_layout, nullptr);

            for (auto &shader : temp.second.shader_modules)
                shader.shutDown();

            vkDestroyPipelineLayout(m_device->logical_device, temp.second.pipeline_layout, nullptr);
            temp.second.pipeline->shutDown();
            delete temp.second.pipeline;
        }

        for (auto &l : m_lights)
            l.shutDown();

        for (auto &m : m_models)
            m.shutDown();

        for (auto &c : m_cameras)
            c.shutDown();

        for (auto &s : m_skyboxes)
            s.shutDown();

        m_scene_uniform_buffer->shutDown();
        delete m_scene_uniform_buffer;

        m_lights_uniform_buffer->shutDown();
        delete m_lights_uniform_buffer;

        vkDestroyDescriptorSetLayout(m_device->logical_device, m_scene_descriptor_set_layout, nullptr);
        vkDestroyDescriptorSetLayout(m_device->logical_device, m_environment_descriptor_set_layout, nullptr);
        vkDestroyDescriptorSetLayout(m_device->logical_device, m_radiance_descriptor_set_layout, nullptr);

        vkDestroyDescriptorPool(m_device->logical_device, m_descriptor_pool, nullptr);

        m_texture_manager->shutDown();
        delete m_texture_manager;

        m_model_manager->shutDown();
        delete m_model_manager;

        m_scene_descriptor_sets.clear();
        m_lights.clear();
        m_models.clear();
        m_cameras.clear();
        m_skyboxes.clear();
    }


    Light* Scene::addLight(glm::vec4 irradiance, float radius)
    {
        VV_ASSERT(m_lights.size() < VV_MAX_LIGHTS, "ERROR: number of lights exceeds VV_MAX_LIGHTS.");
        Light light;
        light.create(irradiance, radius);
        m_lights.push_back(light);
        return &m_lights[m_lights.size() - 1];
    }


    Model* Scene::addModel(std::string path, std::string name, std::string material_template)
    {
        VV_ASSERT(m_initialized, "ERROR: scene needs to be initialized before adding models");
        VV_ASSERT(material_templates.find(material_template) != material_templates.end(), "ERROR: material_template does not exist");
        m_models.emplace_back(Model());
        m_model_manager->loadModel(path, name, &material_templates[material_template], &m_models[m_models.size() - 1]);
        return &m_models[m_models.size() - 1];
    }


    Camera* Scene::addCamera(float fov_y, float near_plane, float far_plane)
    {
        VV_ASSERT(m_initialized, "ERROR: scene needs to be initialized before adding cameras");
        Camera camera;
        camera.create(fov_y, near_plane, far_plane);
        m_cameras.push_back(camera);
        return &m_cameras[m_cameras.size() - 1];
    }


    SkyBox* Scene::addSkyBox(std::string path, std::string radiance_map_name, std::string diffuse_map_name,
                             std::string specular_map_name, std::string brdf_lut_name)
    {
        VV_ASSERT(m_initialized, "ERROR: scene needs to be initialized before adding skyboxes");
        m_skyboxes.emplace_back();

        path = Settings::inst()->getTextureDirectory() + path;
        auto radiance_map = m_texture_manager->loadCubeMap(path, radiance_map_name, VK_FORMAT_R32G32B32A32_SFLOAT, false);
        auto diffuse_map = m_texture_manager->loadCubeMap(path, diffuse_map_name, VK_FORMAT_R32G32B32A32_SFLOAT, true);
        auto specular_map = m_texture_manager->loadCubeMap(path, specular_map_name, VK_FORMAT_R32G32B32A32_SFLOAT, true);
        auto brdf_lut = m_texture_manager->load2DImage(path, brdf_lut_name, VK_FORMAT_R32G32_SFLOAT, false);
        auto sphere_mesh = m_model_manager->getSphereMesh();

        m_skyboxes[m_skyboxes.size() - 1].create(m_device, m_radiance_descriptor_set, m_environment_descriptor_set, sphere_mesh, radiance_map, diffuse_map, specular_map, brdf_lut);
        return &m_skyboxes[m_skyboxes.size() - 1];
    }


    Camera* Scene::getActiveCamera() const
    {
        return m_active_camera;
    }


    SkyBox* Scene::getActiveSkyBox() const
    {
        return m_active_skybox;
    }


    void Scene::setActiveCamera(Camera *camera)
    {
        m_has_active_camera = true;
        m_active_camera = camera;
    }


    void Scene::setActiveSkyBox(SkyBox *skybox)
    {
        m_has_active_skybox = true;
        skybox->updateDescriptorSet();
        m_active_skybox = skybox;
    }


    void Scene::updateUniformData(VkExtent2D extent, float delta_time)
    {
        VV_ASSERT(m_active_camera != nullptr, "ERROR: main camera has not been initialized");

        for (auto i = 0; i < m_lights.size(); ++i)
        {
            m_lights_ubo.lights[i].position = glm::vec4(m_lights[i].getPosition(), 0.0f);
            m_lights_ubo.lights[i].irradiance = m_lights[i].irradiance;
        }
        m_lights_uniform_buffer->updateAndTransfer(&m_lights_ubo);

        m_scene_ubo.view_mat = m_active_camera->getViewMatrix();
        m_scene_ubo.projection_mat = m_active_camera->getProjectionMatrix(extent.width / static_cast<float>(extent.height));
        m_scene_ubo.camera_position = glm::vec4(m_active_camera->getPosition(), 1.0);
        m_scene_uniform_buffer->updateAndTransfer(&m_scene_ubo);

        for (auto &m : m_models)
            m.updateModelUBO();
    }


    void Scene::render(VkCommandBuffer command_buffer)
    {
        bool first_run = true;
        MaterialTemplate *curr_template = nullptr;

        if (m_has_active_skybox)
        {
            auto skybox_template = material_templates["skybox"];
            skybox_template.pipeline->bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skybox_template.pipeline_layout, 0, 1, &m_scene_descriptor_sets[0], 0, nullptr);

            m_active_skybox->bindSkyBoxDescriptorSets(command_buffer, skybox_template.pipeline_layout);
            m_active_skybox->render(command_buffer);
        }

        int i = 0;
        for (auto &model : m_models)
        {
            // reduce pipeline state switches as much as possible
            if (first_run || (curr_template->name != model.material_template->name))
            {
                first_run = false;
                curr_template = model.material_template;
                curr_template->pipeline->bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            }

            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, curr_template->pipeline_layout, 0, 1, &m_scene_descriptor_sets[i++], 0, nullptr);

            // Bind environment lighting descriptor sets
            if (model.material_template->uses_environment_lighting)
            {
                m_active_skybox->bindIBLDescriptorSets(command_buffer, curr_template->pipeline_layout);
                m_active_skybox->submitMipLevelPushConstants(command_buffer, curr_template->pipeline_layout);
            }

            // Render all submeshes within this model
            for (auto &mesh : m_model_manager->m_loaded_meshes[model.m_data_handle])
            {
                Material *material = m_model_manager->m_loaded_materials[model.m_data_handle][model.m_material_id_set][mesh->material_id];
                material->bindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
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

        // todo: shouldn't use this file parsing anymore. should just hardcode shader construction
        // loop through required shaders in info file and initialize them.
        std::string curr_shader_name;
        while (std::getline(file, curr_shader_name))
        {
            MaterialTemplate material_template;
            material_template.name = curr_shader_name; // note: apply name to template based on name assigned to spriv shader

            // Construct shader

            // todo: this removes all generality of this function. should place somewhere else.
            material_template.shader_modules.emplace_back();
            material_template.shader_modules[0].create(m_device, material_template.name, "vert", "main");
            material_template.shader_modules[0].entrance_function = "main";

            material_template.shader_modules.emplace_back();
            material_template.shader_modules[1].create(m_device, material_template.name, "frag", "main");
            material_template.shader_modules[1].entrance_function = "main";

            material_template.uses_environment_lighting = material_template.shader_modules[1].uses_environmental_lighting;

            // Construct Descriptor Set Layouts
            std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
            descriptor_set_layouts.push_back(m_scene_descriptor_set_layout);

            if (curr_shader_name == "skybox")
                descriptor_set_layouts.push_back(m_radiance_descriptor_set_layout);
            else
            {
                std::vector<VkDescriptorSetLayoutBinding> temp_bindings_buffer;

                // material descriptor layout
                for (auto &o : material_template.shader_modules[1].material_descriptor_orderings)
                    temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(o.binding, o.type, 1, o.shader_stage));

                createVulkanDescriptorSetLayout(m_device->logical_device, temp_bindings_buffer, material_template.material_descriptor_set_layout);
                descriptor_set_layouts.push_back(material_template.material_descriptor_set_layout);

                // environment descriptor set layout
                if (material_template.uses_environment_lighting)
                    descriptor_set_layouts.push_back(m_environment_descriptor_set_layout);
            }

            // Construct Pipeline
            VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
            pipeline_layout_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.flags                   = 0;
            pipeline_layout_create_info.setLayoutCount          = static_cast<uint32_t>(descriptor_set_layouts.size());
            pipeline_layout_create_info.pSetLayouts             = descriptor_set_layouts.data();
            pipeline_layout_create_info.pPushConstantRanges     = material_template.shader_modules[1].push_constant_ranges.data();
            pipeline_layout_create_info.pushConstantRangeCount  = material_template.shader_modules[1].push_constant_ranges.size();

            VV_CHECK_SUCCESS(vkCreatePipelineLayout(m_device->logical_device, &pipeline_layout_create_info, nullptr, &material_template.pipeline_layout));

            VulkanPipeline *pipeline = new VulkanPipeline();
            pipeline->createGraphicsPipeline(m_device, material_template.pipeline_layout, m_render_pass);
            pipeline->addShaderStage(material_template.shader_modules[0]);
            pipeline->addShaderStage(material_template.shader_modules[1]);
            pipeline->addVertexInputState();
            pipeline->addInputAssemblyState();
            pipeline->addDepthStencilState(true, true);
            pipeline->addViewportState();
            pipeline->addMultisampleState();
            pipeline->addColorBlendState();

            if (curr_shader_name == "skybox")
                pipeline->addRasterizationState(VK_FRONT_FACE_CLOCKWISE);
            else
                pipeline->addRasterizationState(VK_FRONT_FACE_COUNTER_CLOCKWISE);

            pipeline->commitGraphicsPipeline();
            material_template.pipeline = pipeline;

            // Finished
            material_templates[material_template.name] = material_template;
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
        VV_CHECK_SUCCESS(vkCreateDescriptorPool(m_device->logical_device, &create_info, nullptr, &m_descriptor_pool));
    }


    void Scene::createSceneDescriptorSetLayout()
    {
        // MVP matrix data
        m_scene_ubo = { glm::mat4(), glm::mat4(), glm::vec4() };
        m_scene_uniform_buffer = new VulkanBuffer();
        m_scene_uniform_buffer->create(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SceneUBO));

        // Lights data
        for (auto i = 0; i < VV_MAX_LIGHTS; ++i)
            m_lights_ubo.lights[i] = { glm::vec4(), glm::vec4() };

        m_lights_uniform_buffer = new VulkanBuffer();
        m_lights_uniform_buffer->create(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(LightUBO));

        /// Layout
        std::vector<VkDescriptorSetLayoutBinding> temp_bindings_buffer;
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT));
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT));
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
        createVulkanDescriptorSetLayout(m_device->logical_device, temp_bindings_buffer, m_scene_descriptor_set_layout);
    }


    void Scene::allocateSceneDescriptorSets()
    {
		VkDescriptorSetAllocateInfo scene_alloc_info = {};
		scene_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		scene_alloc_info.descriptorPool = m_descriptor_pool;
		scene_alloc_info.descriptorSetCount = 1;
		scene_alloc_info.pSetLayouts = &m_scene_descriptor_set_layout;

        m_scene_descriptor_sets.resize(m_models.size());

        for (int i = 0; i < m_models.size(); ++i)
        {
		    VV_CHECK_SUCCESS(vkAllocateDescriptorSets(m_device->logical_device, &scene_alloc_info, &m_scene_descriptor_sets[i]));
            std::array<VkWriteDescriptorSet, 3> write_sets;

		    VkDescriptorBufferInfo scene_buffer_info = {};
		    scene_buffer_info.buffer = m_scene_uniform_buffer->buffer;
		    scene_buffer_info.offset = 0;
		    scene_buffer_info.range = sizeof(SceneUBO);

		    write_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		    write_sets[0].dstSet = m_scene_descriptor_sets[i];
		    write_sets[0].dstBinding = 0;
		    write_sets[0].dstArrayElement = 0;
		    write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		    write_sets[0].descriptorCount = 1; // how many elements to update
		    write_sets[0].pBufferInfo = &scene_buffer_info;
            write_sets[0].pNext = NULL;

            VkDescriptorBufferInfo lights_buffer_info = {};
		    lights_buffer_info.buffer = m_lights_uniform_buffer->buffer;
		    lights_buffer_info.offset = 0;
            lights_buffer_info.range = sizeof(LightUBO);

		    write_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		    write_sets[1].dstSet = m_scene_descriptor_sets[i];
		    write_sets[1].dstBinding = 2;
		    write_sets[1].dstArrayElement = 0;
		    write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		    write_sets[1].descriptorCount = 1;
		    write_sets[1].pBufferInfo = &lights_buffer_info;
            write_sets[1].pNext = NULL;

            VkDescriptorBufferInfo model_buffer_info = {};
		    model_buffer_info.buffer = m_models[i].m_model_uniform_buffer->buffer;
		    model_buffer_info.offset = 0;
		    model_buffer_info.range = sizeof(ModelUBO);

            write_sets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		    write_sets[2].dstSet = m_scene_descriptor_sets[i];
		    write_sets[2].dstBinding = 1;
		    write_sets[2].dstArrayElement = 0;
		    write_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		    write_sets[2].descriptorCount = 1;
		    write_sets[2].pBufferInfo = &model_buffer_info;
            write_sets[2].pNext = NULL;
		
            vkUpdateDescriptorSets(m_device->logical_device, 3, write_sets.data(), 0, nullptr);
        }
    }


    void Scene::createEnvironmentUniforms()
	{
        std::vector<VkDescriptorSetLayoutBinding> temp_bindings_buffer;
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
        createVulkanDescriptorSetLayout(m_device->logical_device, temp_bindings_buffer, m_environment_descriptor_set_layout);

        temp_bindings_buffer.clear();
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
        createVulkanDescriptorSetLayout(m_device->logical_device, temp_bindings_buffer, m_radiance_descriptor_set_layout);

        /// Descriptor Sets
		VkDescriptorSetAllocateInfo env_alloc_info = {};
		env_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		env_alloc_info.descriptorPool = m_descriptor_pool;
		env_alloc_info.descriptorSetCount = 1;
		env_alloc_info.pSetLayouts = &m_environment_descriptor_set_layout;

		VV_CHECK_SUCCESS(vkAllocateDescriptorSets(m_device->logical_device, &env_alloc_info, &m_environment_descriptor_set));

        VkDescriptorSetAllocateInfo rad_alloc_info = {};
		rad_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		rad_alloc_info.descriptorPool = m_descriptor_pool;
		rad_alloc_info.descriptorSetCount = 1;
		rad_alloc_info.pSetLayouts = &m_radiance_descriptor_set_layout;

		VV_CHECK_SUCCESS(vkAllocateDescriptorSets(m_device->logical_device, &rad_alloc_info, &m_radiance_descriptor_set));
	}


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
