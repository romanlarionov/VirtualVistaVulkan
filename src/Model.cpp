
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
        _data_handle = data_handle;
        _material_id_set = material_id_set;

        _model_ubo = { glm::mat4(), glm::mat4() };
        _model_uniform_buffer = new VulkanBuffer();
        _model_uniform_buffer->create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ModelUBO));
	}


	void Model::shutDown()
	{
        _model_uniform_buffer->shutDown();
        delete _model_uniform_buffer;
	}


    void Model::updateModelUBO()
    {
        _model_ubo = { _pose, glm::transpose(glm::inverse(_pose)) };
        _model_uniform_buffer->updateAndTransfer(&_model_ubo);
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
