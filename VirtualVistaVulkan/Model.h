
#ifndef VIRTUALVISTA_MODEL_H
#define VIRTUALVISTA_MODEL_H

#include <vector>

#include "Mesh.h"

namespace vv
{
	struct Model 
	{
	public:
		Model();

		~Model();

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

#endif // VIRTUALVISTA_MODEL_H