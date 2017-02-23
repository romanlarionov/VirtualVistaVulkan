
#ifndef VIRTUALVISTA_MODEL_H
#define VIRTUALVISTA_MODEL_H

#include <string>

#include "VulkanDevice.h"
#include "Entity.h"
#include "Mesh.h"
#include "Material.h"

namespace vv
{
	class Model : public Entity
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
		void create(std::string name, std::string data_handle, std::string material_id_set, MaterialTemplate *material_template);

		/*
		 *
		 */
		void shutDown();

        /*
         * Used for update of model matrix at render time.
         */
        glm::mat4 getModelMatrix() const;
		
	private:
        // note: acts as hash key for ModelManager's data caches. this is used by scene during render-time.
        //       in reality this is simply the path + name of the file originally queried.
        std::string _data_handle;
        std::string _material_id_set;

	};
}

#endif // VIRTUALVISTA_MODEL_H