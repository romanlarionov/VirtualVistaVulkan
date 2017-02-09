
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
		void create(VulkanDevice *device, MaterialTemplate *dummy_template, VkDescriptorPool descriptor_pool, VkSampler sampler);

		/*
		 *
		 */
		void shutDown();

        /*
         * Creates a model using the appropriate model loader based on file extension.
         * This override allows the specification of a custom shader to be used during construction.
         *
         * note: This returns (by address) a COPY of the constructed model. This eliminates a lot of hassle from the
         *       creation process even though it could theoretically be sped up through a more complex caching design.
         */
        bool loadModel(std::string path, std::string name, MaterialTemplate *material_template, Model *model);

    private:
		VulkanDevice *_device;
        MaterialTemplate *_dummy_template;
        VkDescriptorPool _descriptor_pool;
        VkSampler _sampler;

        // todo: consider using smart pointers for reference counting.
        //std::unordered_map<std::string, Model *> _cached_models;
        std::unordered_map<std::string, std::vector<Mesh *> > _loaded_meshes;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Material *> > > _loaded_materials;

        // note: assumes these have been pre-initialized. That way the actual asset loading can be looped through once.
        //std::vector<MaterialTemplate> material_templates_;

        // todo: can have global array of geometry and material data that constantly updates.

        /*
        *
        */
        bool loadOBJ(std::string path, std::string name, bool load_geometry, MaterialTemplate *material_template, Model *model);

        /*
         *
         */
        bool loadGLTF();
	};
}

#endif // VIRTUALVISTA_ASSETMANAGER_H