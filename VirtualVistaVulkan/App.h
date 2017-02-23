
#ifndef VIRTUALVISTA_APP_H
#define VIRTUALVISTA_APP_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "VulkanRenderer.h"
#include "Scene.h"

namespace vv
{
	class App
	{
	public:
		App(int argc, char **argv);
		~App();

        /*
         * Initializes all essential components required by the engine.
         */
		void create();

        /*
         * Must be called at end of execution to ensure all allocated resources are successfully purged.
         */
		void shutDown();

        /*
         * Returns the main scene which manages all entities with physical presence.
         * Entities that should be rendered, cameras, and lights can be added through this central object.
         */
        Scene* getScene() const;

        /*
         * Signals the Vulkan engine to construct render commands and begins all central processing.
         */
		void beginMainLoop(std::function<void(Scene*, float)> input_handler);

	private:
		int _argc;
		char **_argv;

		VulkanRenderer *_renderer;
        Scene *_scene;
	};
}

#endif // VIRTUALVISTA_APP_H