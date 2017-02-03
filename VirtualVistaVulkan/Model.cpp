
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


	void Model::create(std::string name, std::vector<Mesh> &meshes, std::vector<Material> &materials)
	{
        name_ = name;
        meshes_ = meshes;
        material_instances_ = materials;

        std::sort(meshes_.begin(), meshes_.end(), [](const Mesh &l, const Mesh &r) {
            return l.material_id < r.material_id;
        });
	}


	void Model::shutDown()
	{
        for (auto &mesh : meshes_)
            mesh.shutDown();

        for (auto &mat : material_instances_)
            mat.shutDown();
	}


    void Model::renderByMaterial(VkCommandBuffer command_buffer, VkDescriptorSet general_descriptor_set)
    {
        int curr_material_id = -1;
        for (auto &mesh : meshes_)
        {
            Material material = material_instances_[mesh.material_id];
            if (mesh.material_id != curr_material_id) // minimize pipeline context switches
            {
                curr_material_id = mesh.material_id;
                material.material_template->bindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            }

            material.bindDescriptorSets(command_buffer, general_descriptor_set);
            mesh.bindBuffers(command_buffer);
            mesh.render(command_buffer);
        }
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
