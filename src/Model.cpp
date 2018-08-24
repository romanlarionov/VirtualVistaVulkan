
#include "Model.h"
#include "Utils.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Model::Model()
	{
	}


	Model::~Model()
	{
	}


	void Model::create(VulkanDevice *device, std::string name, std::string data_handle, std::string material_id_set,
                       MaterialTemplate *material_template)
	{
        this->name = name;
        this->material_template = material_template;
        m_data_handle = data_handle;
        m_material_id_set = material_id_set;

        m_model_ubo = { glm::mat4(), glm::mat4() };
        m_model_uniform_buffer = new VulkanBuffer();
        m_model_uniform_buffer->create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ModelUBO));
	}


	void Model::shutDown()
	{
        m_model_uniform_buffer->shutDown();
        delete m_model_uniform_buffer;
	}


    void Model::updateModelUBO()
    {
        m_model_ubo = { m_pose, glm::transpose(glm::inverse(m_pose)) };
        m_model_uniform_buffer->updateAndTransfer(&m_model_ubo);
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
