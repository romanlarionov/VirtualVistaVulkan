
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <cstring>

#include "ModelManager.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	ModelManager::ModelManager()
	{
	}


	ModelManager::~ModelManager()
	{
	}


	void ModelManager::create(VulkanDevice *device, TextureManager *texture_manager, VkDescriptorPool descriptor_pool)
	{
		_device = device;
        _texture_manager = texture_manager;
        _descriptor_pool = descriptor_pool;

        // load primitive mesh to cache
        Model *temp_model = new Model();
        loadOBJ(Settings::inst()->getModelDirectory() + "primitives/", "sphere.obj", nullptr, temp_model);
        delete temp_model;
	}


	void ModelManager::shutDown()
	{
        for (auto &m : _loaded_meshes)
            for (auto &mesh : m.second)
            {
                mesh->shutDown();
                delete mesh;
            }

        for (auto &m : _loaded_materials)
            for (auto &mat : m.second)
                for (auto & material : mat.second)
                {
                    material->shutDown();
                    delete material;
                }
	}


    bool ModelManager::loadModel(std::string path, std::string name, MaterialTemplate *material_template, Model *model)
    {
        bool load_geometry = true;
        path = Settings::inst()->getModelDirectory() + path;
        std::string file_type = name.substr(name.find_first_of('.') + 1);

        // check if geometry has already been loaded
        if (_loaded_meshes.count(path + name) > 0)
        {
            // materials have been loaded as well
            if (_loaded_materials[path + name].count(material_template->name) > 0)
            {
                auto mat_templ = _loaded_materials[path + name][material_template->name][0]->material_template;
                model->create(_device, "", path + name, material_template->name, mat_templ);
                return true;
            }
            else
                load_geometry = false;
        }

        if (file_type == "obj")
            return loadOBJ(path, name, material_template, model);

        else if (file_type == "gltf")
            return loadGLTF();

        else
        {
            VV_ASSERT(false, "File type: " + file_type + " not supported");
            return false;
        }
    }


    Mesh* ModelManager::getSphereMesh() const
    {
        return _loaded_meshes.at(Settings::inst()->getModelDirectory() + "primitives/sphere.obj")[0];
    }


    ///////////////////////////////////////////////////////////////////////////////////////////// Private
    bool ModelManager::loadOBJ(std::string path, std::string name, MaterialTemplate *material_template, Model *model)
    {
        bool success = true;
        std::string full_path(path + name);
    	tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> tiny_shapes;
		std::vector<tinyobj::material_t> tiny_materials;
		std::string err;

        std::vector<Mesh *> meshes;
        std::vector<Material *> materials;

		VV_ASSERT(tinyobj::LoadObj(&attrib, &tiny_shapes, &tiny_materials, &err,
                  full_path.c_str(), path.c_str()),
                  "Model, " + name + ", not loaded correctly\n\n" + err);

        // parse through all loaded geometry and create internal abstractions.
		for (const auto& shape : tiny_shapes)
		{
		    std::vector<Vertex> vertices;
		    std::vector<uint32_t> indices;
		    std::unordered_map<Vertex, int> vertex_map;

			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex = {};

                // Vertices
				vertex.position = glm::vec3(
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				);

                // Normals
                if (!attrib.normals.empty())
                    vertex.normal = glm::vec3(
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    );
                else
                {
				    vertex.normal = glm::vec3(0.0, 0.0, 1.0);
                    VV_ALERT("Model does not have normals.");
                }

                // UVs
                if (!attrib.texcoords.empty())
                    vertex.texCoord = glm::vec2(
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    );
                else
                {
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);
                    VV_ALERT("Model does not have UV coordinates.");
                }

				if (vertex_map.count(vertex) == 0)
				{
					vertex_map[vertex] = (int)vertices.size();
					vertices.push_back(vertex);
				}

				indices.push_back(vertex_map[vertex]);
			}

            int curr_material_id = shape.mesh.material_ids[0];

            Mesh *mesh = new Mesh();
            mesh->create(_device, shape.name, vertices, indices, ((curr_material_id < 0) ? 0 : curr_material_id));
            meshes.push_back(mesh);
		}

        _loaded_meshes[path + name] = meshes;

        if (material_template)
        {
            // parse through all loaded materials and create internal abstractions.
            for (const auto &m : tiny_materials)
            {
                Material *material = new Material();
                material->create(_device, material_template, _descriptor_pool);

                // store required descriptor set data in correct binding order
                auto orderings = material_template->shader_modules[1].material_descriptor_orderings;
                for (size_t i = 0; i < orderings.size(); ++i)
                {
                    auto o = orderings[i];
                    if (o.name == "properties")
                    {
                        glm::vec4 amb(m.ambient[0], m.ambient[1], m.ambient[2], 0.0);
                        glm::vec4 dif(m.diffuse[0], m.diffuse[1], m.diffuse[2], 0.0);
                        glm::vec4 spec(m.specular[0], m.specular[1], m.specular[2], 0.0);
                        MaterialProperties properties = { amb, dif, spec, static_cast<int>(m.shininess) };

                        VulkanBuffer *buffer = new VulkanBuffer();
                        buffer->create(_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(properties));
                        buffer->updateAndTransfer(&properties);

                        material->addUniformBuffer(buffer, o.binding);
                    }
                    else if (o.name.find("map") != std::string::npos)
                    {
                        std::string temp_name;
                        if (o.name == "ambient_map")
                            temp_name = m.ambient_texname;
                        else if (o.name == "diffuse_map")
                            temp_name = m.diffuse_texname;
                        else if (o.name == "specular_map")
                            temp_name = m.specular_texname;
                        else if (o.name == "normal_map")
                            temp_name = m.normal_texname;
                        else if (o.name == "roughness_map")
                            temp_name = m.roughness_texname;
                        else if (o.name == "metalness_map")
                            temp_name = m.metallic_texname;
                        else if (o.name == "emissiveness_map")
                            temp_name = m.emissive_texname;

                        // todo: need to support more texture types
                        auto texture = _texture_manager->load2DImage(path, temp_name, VK_FORMAT_R8G8B8A8_UNORM, false);
                        material->addTexture(texture, o.binding);
                    }
                    else // descriptor type not populated
                    {
                        VV_ALERT("WARNING: Descriptor Type not populated for model: " + name + ". Using dummy material.");
                        success = false;
                        break;
                    }
                }

                material->updateDescriptorSets();
                materials.push_back(material);
            }

            // if no mtl file was found
            if (tiny_materials.empty())
            {
                Material *material = new Material();
                material->create(_device, material_template, _descriptor_pool);

                //VV_ALERT("MTL file not found. Assuming PBR textures present.");
                
                // todo: hardcoded access to fragment shader here. need to remove
                auto orderings = material_template->shader_modules[1].material_descriptor_orderings;
                for (size_t i = 0; i < orderings.size(); ++i)
                {
                    auto o = orderings[i];
                    std::string temp_name;

                    if (o.name == "normal_map")
                        temp_name = "normal.dds";
                    else if (o.name == "albedo_map")
                        temp_name = "albedo.dds";
                    else if (o.name == "roughness_map")
                        temp_name = "roughness.dds";
                    else if (o.name == "metalness_map")
                        temp_name = "metalness.dds";
                    else if (o.name == "emissiveness_map")
                        temp_name = "emissiveness.dds";
                    else if (o.name == "ambient_occlusion_map")
                        temp_name = "ambient_occlusion.dds";

                    auto texture = _texture_manager->load2DImage(path + "textures/", temp_name);
                    material->addTexture(texture, o.binding);
                }

                material->updateDescriptorSets();
                materials.push_back(material);
            }

            _loaded_materials[path + name][material_template->name] = materials;
            model->create(_device, name, path + name, material_template->name, material_template);
        }

        return success;
    }


    bool ModelManager::loadGLTF()
    {
        return false;
    }
}
