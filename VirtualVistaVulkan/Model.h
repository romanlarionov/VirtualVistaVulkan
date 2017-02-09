
#ifndef VIRTUALVISTA_MODEL_H
#define VIRTUALVISTA_MODEL_H

#include <string>
#include <vector>

#include "VulkanDevice.h"
#include "Mesh.h"
#include "Material.h"

namespace vv
{
	class Model 
	{
        friend class Scene;

	public:
		std::string name;
        MaterialTemplate *material_template;

		Model();
		~Model();

		/*
         * Stores all necessary components for a loaded model including geometry and material data.
         * Creation should be left to the AssetManager class which handles loading and managing assets such as this.
		 */
		//void create(std::string name, std::vector<Mesh *> &meshes, std::vector<Material *> &materials);
		void create(std::string name, std::string data_handle, std::string material_id_set, MaterialTemplate *material_template);
		
		/*
		 *
		 */
		void shutDown();
		
        /*
         *
         */
        //void renderByMaterial(VkCommandBuffer command_buffer, std::vector<VkDescriptorSet> descriptor_sets);

	private:
        //std::vector<Mesh *> _meshes;
        //std::vector<Material *> _material_instances;

        // note: acts as hash key for ModelManager's data caches. this is used by scene during render-time.
        //       in reality this is simply the path + name of the file originally queried.
        std::string _data_handle;
        std::string _material_id_set;
	};
}

#endif // VIRTUALVISTA_MODEL_H