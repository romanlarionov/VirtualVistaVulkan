
#ifndef VIRTUALVISTA_ASSETMANAGER_H
#define VIRTUALVISTA_ASSETMANAGER_H

#include <unordered_map>
#include <string>
#include <vector>

#include "VulkanRenderPass.h"
#include "VulkanDevice.h"
#include "Shader.h"
#include "Model.h"
#include "Mesh.h"
#include "Material.h"

namespace vv
{
	class ModelManager
	{
        friend class Scene;

	public:
        int material_instance_count;

		ModelManager();
		~ModelManager();

		/*
		 * High level class that handles all asset loading, initialization, and management.
		 */
		void create(VulkanDevice *device, VkDescriptorPool descriptor_pool, VkSampler sampler);

		/*
		 *
		 */
		void shutDown();

        /*
         * Creates a model using the appropriate model loader based on file extension.
         * The provided MaterialTemplate will be used to load the required descriptors with data found in the path.
         */
        bool loadModel(std::string path, std::string name, MaterialTemplate *material_template, Model *model);

    private:
		VulkanDevice *_device;
        VkDescriptorPool _descriptor_pool;
        VkSampler _sampler;

        // todo: consider using smart pointers for reference counting.
        std::unordered_map<std::string, std::vector<Mesh *> > _loaded_meshes;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Material *> > > _loaded_materials;

        // todo: can have global array of geometry and material data that constantly updates.

        /*
         * Loads obj + mtl files for a single model. Returns a model abstraction with references to raw loaded geometry + material data.
         */
        bool loadOBJ(std::string path, std::string name, bool load_geometry, MaterialTemplate *material_template, Model *model);

        /*
         * todo: add support for glTF
         */
        bool loadGLTF();
	};
}

#endif // VIRTUALVISTA_ASSETMANAGER_H