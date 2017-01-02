
#ifndef VIRTUALVISTA_SCENE_H
#define VIRTUALVISTA_SCENE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "Light.h"
#include "Model.h"
#include "Camera.h"

namespace vv
{
	typedef std::string Handle;

	struct Scene
	{
	public:
		Scene();

		~Scene();

		/*
		 * 
		 */
		void create();

		/*
		 *
		 */
		void shutDown();

		/*
		 * Creates a duplicate copy of an existing object within the scene, adds it to the scene with a unique handle, and returns the handle for access.
		 */
		Handle cloneLight(Handle handle);
		Handle cloneModel(Handle handle);
		Handle cloneCamera(Handle handle);

		/*
		 * Creates an entity that holds presence in the scene and returns a handle for access.
		 * todo: figure out what parameters need to be passed in here.
		 */
		Handle createLight();
		Handle createModel();
		Handle createCamera();

		/*
		 * Removes the given entity from the scene permanently.
		 */
		void destroyLight(Handle handle);
		void destroyModel(Handle handle);
		void destroyCamera(Handle handle);

		
	private:

		std::unordered_map<Handle, Light*> lights_;
		std::unordered_map<Handle, Model*> models_;
		std::unordered_map<Handle, Camera*> cameras_;
	};
}

#endif // VIRTUALVISTA_SCENE_H