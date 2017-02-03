
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
	public:
		Model();
		~Model();

		/*
         * Stores all necessary components for a loaded model including geometry and material data.
         * Creation should be left to the AssetManager class which handles loading and managing assets such as this.
		 */
		void create(std::string name, std::vector<Mesh> &meshes, std::vector<Material> &materials);
		
		/*
		 *
		 */
		void shutDown();
		
        /*
         *
         */
        void renderByMaterial(VkCommandBuffer command_buffer, VkDescriptorSet descriptor_set);

	private:
		std::string name_;
        std::vector<Mesh> meshes_;
        std::vector<Material> material_instances_;

	};
}

#endif // VIRTUALVISTA_MODEL_H