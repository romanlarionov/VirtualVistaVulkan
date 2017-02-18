
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


	void ModelManager::create(VulkanDevice *device, VkDescriptorPool descriptor_pool, VkSampler sampler)
	{
		_device = device;
        _descriptor_pool = descriptor_pool;
        _sampler = sampler;

        // todo: construct primative geometry
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
        // todo: should pass descriptor sets that are general to model and pass those to model class.

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
                model->create("", path + name, material_template->name, mat_templ);
                return true;
            }
            else
                load_geometry = false;
        }

        if (file_type == "obj")
        {
            return loadOBJ(path, name, load_geometry, material_template, model);
        }
        else if (file_type == "gltf")
        {
            return loadGLTF();
        }
        else
        {
            VV_ASSERT(false, "File type: " + file_type + " not supported");
            return false;
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////////////// Private
    bool ModelManager::loadOBJ(std::string path, std::string name, bool load_geometry, MaterialTemplate *material_template, Model *model)
    {
        bool success = true;
        std::string full_path(path + name);
    	tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> tiny_shapes;
		std::vector<tinyobj::material_t> tiny_materials;
		std::string err;

        std::vector<Mesh *> meshes;
        std::vector<Material *> materials;

        // todo: add branch for when geometry has already been loaded. need to figure out how LoadMtl works

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

				vertex.color = glm::vec3(1.0, 1.0, 1.0);
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

				indices.push_back(vertex_map[vertex]);
			}

            int curr_material_id = shape.mesh.material_ids[0];

            // todo: construct mesh from parsed data
            Mesh *mesh = new Mesh();
            mesh->create(_device, shape.name, vertices, indices, ((curr_material_id < 0) ? 0 : curr_material_id));
            meshes.push_back(mesh);
		}

        _loaded_meshes[path + name] = meshes;

        // parse through all loaded materials and create internal abstractions.
        for (const auto &m: tiny_materials)
        {
            Material *material = new Material();
            material->create(_device, material_template, _descriptor_pool);

            // store required descriptor set data in correct binding order
            //auto orderings = material_template->descriptor_orderings;
            auto orderings = material_template->shader->standard_material_descriptor_orderings;
            for (size_t i = 0; i < orderings.size(); ++i)
            {
                auto o = orderings[i];
                if (o.name == "constants")
                {
                    // todo: check if these are always present. for now assuming they are.
                    glm::vec4 amb(m.ambient[0], m.ambient[1], m.ambient[2], 0.0);
                    glm::vec4 dif(m.diffuse[0], m.diffuse[1], m.diffuse[2], 0.0);
                    glm::vec4 spec(m.specular[0], m.specular[1], m.specular[2], 0.0);
                    MaterialConstants constants = { amb, dif, spec, static_cast<int>(m.shininess) };

                    VulkanBuffer *buffer = new VulkanBuffer();
                    buffer->create(_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(constants));
                    buffer->updateAndTransfer(&constants);

                    material->addUniformBuffer(buffer, static_cast<int>(i));
                }
                else if (o.name == "ambient_texture")
                {
                    VulkanImage *texture = new VulkanImage();
                    if (m.ambient_texname != "")
                        texture->createColorAttachment(path + m.ambient_texname, _device, VK_FORMAT_R8G8B8A8_UNORM);
                    else
                        texture->createColorAttachment(Settings::inst()->getAssetDirectory() + "dummy.png", _device, VK_FORMAT_R8G8B8A8_UNORM);

                    texture->transferToDevice();
                    material->addTexture(texture, static_cast<int>(i), _sampler);
                }
                else if (o.name == "diffuse_texture")
                {
                    VulkanImage *texture = new VulkanImage();
                    if (m.diffuse_texname != "")
                        texture->createColorAttachment(path + m.diffuse_texname, _device, VK_FORMAT_R8G8B8A8_UNORM);
                    else
                        texture->createColorAttachment(Settings::inst()->getAssetDirectory() + "dummy.png", _device, VK_FORMAT_R8G8B8A8_UNORM);

                    texture->transferToDevice();
                    material->addTexture(texture, static_cast<int>(i), _sampler);
                }
                else if (o.name == "specular_texture")
                {
                    VulkanImage *texture = new VulkanImage();
                    if (m.specular_texname != "")
                        texture->createColorAttachment(path + m.specular_texname, _device, VK_FORMAT_R8G8B8A8_UNORM);
                    else
                        texture->createColorAttachment(Settings::inst()->getAssetDirectory() + "dummy.png", _device, VK_FORMAT_R8G8B8A8_UNORM);
                    
                    texture->transferToDevice();
                    material->addTexture(texture, static_cast<int>(i), _sampler);
                }
                else // descriptor type not populated
                {
                    // todo: remove and fill with dummy data as opposed to using different material template
                    std::cerr << "ERROR: Descriptor Type not populated for model: " << name << ". Using dummy material.\n";
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

            // todo: this should loop over all non-standard descriptors provided
            materials.push_back(material);
        }

        _loaded_materials[path + name][material_template->name] = materials;

        model->create(name, path + name, material_template->name, material_template);
        return success;
    }


    bool ModelManager::loadGLTF()
    {
        return false;
    }
}
