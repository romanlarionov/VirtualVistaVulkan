
#include "Light.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Light::Light()
	{
	}


	Light::~Light()
	{
	}


	void Light::create(glm::vec4 irradiance, float radius)
	{
        irradiance.a = radius;
        this->irradiance = irradiance;
        this->radius = radius;
	}


	void Light::shutDown()
	{

	}

	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
