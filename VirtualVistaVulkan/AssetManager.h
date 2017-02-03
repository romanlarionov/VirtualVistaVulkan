
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
	class AssetManager
	{
	public:
        int material_instance_count;

		AssetManager();
		~AssetManager();

		/*
		 * High level class that handles all asset loading, initialization, and management.
		 */
		void create(VulkanDevice *device, std::vector<MaterialTemplate> material_templates);

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
        bool loadModel(std::string path, std::string name, Model &model);

    private:
		VulkanDevice *device_;

        // todo: consider using smart pointers for reference counting.
        std::unordered_map<std::string, Model> cached_models_;

        // note: assumes these have been pre-initialized. That way the actual asset loading can be looped through once.
        std::vector<MaterialTemplate> material_templates_;

        // todo: can have global array of geometry and material data that constantly updates.

        /*
        *
        */
        bool loadOBJ(std::string path, std::string name, Model &model);

        /*
         *
         */
        bool loadGLTF();
	};
}

#endif // VIRTUALVISTA_ASSETMANAGER_H