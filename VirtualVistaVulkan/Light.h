
#ifndef VIRTUALVISTA_LIGHT_H
#define VIRTUALVISTA_LIGHT_H

#include <vector>

#include "Entity.h"

namespace vv
{
	class Light : public Entity
	{
	public:
		Light();
		~Light();

		/*
		 * 
		 */
		void create();

		/*
		 *
		 */
		void shutDown();
		
	private:
	};
}

#endif // VIRTUALVISTA_LIGHT_H