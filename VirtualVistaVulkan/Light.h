
#ifndef VIRTUALVISTA_LIGHT_H
#define VIRTUALVISTA_LIGHT_H

#include <vector>

#include "Entity.h"

namespace vv
{
	class Light : public Entity
	{
	public:
        glm::vec4 irradiance;
        float radius;

		Light();
		~Light();

		/*
		 * Creates a point light.
		 */
		void create(glm::vec4 irradiance, float radius);

		/*
		 *
		 */
		void shutDown();
		
	private:
	};
}

#endif // VIRTUALVISTA_LIGHT_H