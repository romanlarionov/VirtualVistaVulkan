
#include "Model.h"
#include "Utils.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Model::Model()
	{
	}


	Model::~Model()
	{
	}


	//void Model::create(std::string name, std::vector<Mesh *> &meshes, std::vector<Material *> &materials)
	void Model::create(std::string name, std::string data_handle, std::string material_id_set, MaterialTemplate *material_template)
	{
        this->name = name;
        this->material_template = material_template;
        _data_handle = data_handle;
        _material_id_set = material_id_set;

        /*std::sort(_meshes.begin(), _meshes.end(), [](const Mesh *l, const Mesh *r) {
            return l->material_id < r->material_id;
        });*/
	}


	void Model::shutDown()
	{
        /*for (auto &mesh : _meshes)
            mesh->shutDown();

        for (auto &mat : _material_instances)
            mat->shutDown();*/
	}


    /*void Model::renderByMaterial(VkCommandBuffer command_buffer, std::vector<VkDescriptorSet> descriptor_sets)
    {
        int curr_material_id = -1;
        for (auto &mesh : _meshes)
        {
            Material *material = _material_instances[mesh->material_id];
            if (mesh->material_id != curr_material_id) // minimize pipeline context switches
            {
                curr_material_id = mesh->material_id;
                material->material_template->bindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            }

            material->bindDescriptorSets(command_buffer, descriptor_sets);
            mesh->bindBuffers(command_buffer);
            mesh->render(command_buffer);
        }
    }*/


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
