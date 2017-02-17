
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


	void Model::create(std::string name, std::string data_handle, std::string material_id_set, MaterialTemplate *material_template)
	{
        this->name = name;
        this->material_template = material_template;
        _data_handle = data_handle;
        _material_id_set = material_id_set;
	}


	void Model::shutDown()
	{
  
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
