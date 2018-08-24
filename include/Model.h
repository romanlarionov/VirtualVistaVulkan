
#ifndef VIRTUALVISTA_MODEL_H
#define VIRTUALVISTA_MODEL_H

#include <string>

#include "VulkanDevice.h"
#include "Entity.h"
#include "Mesh.h"
#include "Material.h"

namespace vv
{
    struct ModelUBO
    {
        glm::mat4 model_mat;
        glm::mat4 normal_mat;
    };

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
         * 
         * note: creation should be left to the ModelManager class which handles loading and managing assets such as this.
		 */
		void create(VulkanDevice *device, std::string name, std::string data_handle, std::string material_id_set,
                    MaterialTemplate *material_template);

		/*
		 * This class does not hold ownership of any actual raw data.
		 */
		void shutDown();

        /*
         * Used for update of model + normal matrix at render time.
         */
        void updateModelUBO();
		
	private:
        // note: acts as hash key for ModelManager's data caches. this is used by scene during render-time.
        //       in reality this is simply the path + name of the file originally queried.
        std::string m_data_handle;
        std::string m_material_id_set;

        ModelUBO m_model_ubo;
		VulkanBuffer *m_model_uniform_buffer = nullptr;

	};
}

#endif // VIRTUALVISTA_MODEL_H