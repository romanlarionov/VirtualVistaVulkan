
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "AssetManager.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	AssetManager::AssetManager()
	{
	}


	AssetManager::~AssetManager()
	{
	}


	void AssetManager::create(VulkanDevice *device, std::vector<MaterialTemplate> material_templates)
	{
		device_ = device;
        material_templates_ = material_templates;

        // todo: construct default material
        // todo: construct primative geometry
	}


	void AssetManager::shutDown()
	{
	}


    bool AssetManager::loadModel(std::string path, std::string name, Model &model)
    {
        path = Settings::inst()->getModelDirectory() + path;
        std::string file_type = name.substr(name.find_first_of('.') + 1);

        // check if model has already been loaded.
        if (cached_models_.count(path + name) > 0)
        {
            model = cached_models_[path + name];
            return true;
        }

        if (file_type == "obj")
        {
            return loadOBJ(path, name, model);
        }
        else if (file_type == "gltf")
        {
            return loadGLTF();
        }
        else
        {
            VV_ASSERT(false, "File type provided not supported");
            return false;
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////////////// Private
    bool AssetManager::loadOBJ(std::string path, std::string name, Model &model)
    {
        std::string full_path(path + name);
    	tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> tiny_shapes;
		std::vector<tinyobj::material_t> tiny_materials;
		std::string err;

		VV_ASSERT(tinyobj::LoadObj(&attrib, &tiny_shapes, &tiny_materials, &err,
                  full_path.c_str(), path.c_str()),
                  "Model, " + name + ", not loaded correctly\n\n" + err);

        std::vector<Mesh> meshes;
        std::vector<Material> materials;

        // parse through all loaded geometry and create internal abstractions.
		for (const auto& shape : tiny_shapes)
		{
		    std::vector<Vertex> vertices;
		    std::vector<uint32_t> indices;
		    std::unordered_map<Vertex, int> vertex_map;

			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex = {};

				vertex.position = glm::vec3(
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				);

                if (!attrib.texcoords.empty())
                {
                    vertex.texCoord = glm::vec2(
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    );
                }
                else
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);

				if (vertex_map.count(vertex) == 0)
				{
					vertex_map[vertex] = (int)vertices.size();
					vertices.push_back(vertex);
				}

				vertex.color = glm::vec3(1.0, 1.0, 1.0);

				indices.push_back(vertex_map[vertex]);
			}

            int curr_material_id = shape.mesh.material_ids[0];

            // todo: construct mesh from parsed data
            Mesh mesh;
            mesh.create(device_, shape.name, vertices, indices, curr_material_id);
            meshes.push_back(mesh);
		}

        // parse through all loaded materials and create internal abstractions.
        for (const auto &m: tiny_materials)
        {
            // todo: construct material template if one not found, else return instance of material from cached template
            // todo: if no material is found or if it corrupted, load a default material and log error

            bool template_found = false;
            Material material;

            // iterativly search through templates for comparison.
            for (auto &material_template : material_templates_)
            {
                if (m.name == material_template.name)
                {
                    material.create(&material_template);

                    // store required descriptor set data in correct binding order
                    auto orderings = material.getDescriptorOrdering();
                    for (size_t i = 0; i < orderings.size(); ++i)
                    {
                        auto o = orderings[i];
                        if (o == DescriptorType::CONSTANTS) // todo: check if these are always present. for now assuming they are.
                        {
                            glm::vec3 amb(m.ambient[0], m.ambient[1], m.ambient[2]);
                            glm::vec3 dif(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
                            glm::vec3 spec(m.specular[0], m.specular[1], m.specular[2]);
                            int shin = m.shininess;
                            MaterialConstants constants = { amb, dif, spec, shin };

                            VulkanBuffer *buffer = new VulkanBuffer();
                            buffer->create(device_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(MaterialConstants), 1);
                            buffer->updateAndTransfer(&constants);

                            material.addUniformBuffer(buffer, static_cast<int>(i));
                        }
                        else if (o == DescriptorType::AMBIENT_MAP && m.ambient_texopt.type != tinyobj::texture_type_t::TEXTURE_TYPE_NONE)
                        {
                            VulkanImage *texture = new VulkanImage();
                            texture->createColorAttachment(path + m.ambient_texname, device_, VK_FORMAT_R8G8B8A8_UNORM);
                            material.addTexture(texture, static_cast<int>(i));
                        }
                        else if (o == DescriptorType::DIFFUSE_MAP && m.diffuse_texopt.type != tinyobj::texture_type_t::TEXTURE_TYPE_NONE)
                        {
                            VulkanImage *texture = new VulkanImage();
                            texture->createColorAttachment(path + m.diffuse_texname, device_, VK_FORMAT_R8G8B8A8_UNORM); // todo: test texname. what value will it be?
                            material.addTexture(texture, static_cast<int>(i));
                        }
                        else if (o == DescriptorType::SPECULAR_MAP && m.specular_texopt.type != tinyobj::texture_type_t::TEXTURE_TYPE_NONE)
                        {
                            VulkanImage *texture = new VulkanImage();
                            texture->createColorAttachment(path + m.specular_texname, device_, VK_FORMAT_R8G8B8A8_UNORM);
                            material.addTexture(texture, static_cast<int>(i));
                        }
                        else // descriptor type not populated
                        {
                            std::cerr << "ERROR: Descriptor Type not populated for model: " << name << ". Using dummy material.\n";
                            goto end_loop;
                        }
                    }

                    material.updateDescriptorSets();
                    template_found = true;
                    break;
                }
            }
end_loop:

            /*if (!template_found)
            {
                // load dummy texture, log
                material.create(<dummy>);
            }*/

            materials.push_back(material);
            material_instance_count++;
        }

        model.create(name, meshes, materials);
        cached_models_[name] = model;

        return true;
    }


    bool AssetManager::loadGLTF()
    {
        return false;
    }
}
