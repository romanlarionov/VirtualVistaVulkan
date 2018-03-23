
#ifndef VIRTUALVISTA_VIRTUALVISTAENGINE_H
#define VIRTUALVISTA_VIRTUALVISTAENGINE_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Renderer.h"
//#include "VulkanForwardRenderer.h"
#include "GLFWWindow.h"
#include "Scene.h"

namespace vv
{
	class VirtualVistaEngine
	{
	public:
		VirtualVistaEngine(int argc, char **argv);
		~VirtualVistaEngine();

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
		void beginMainLoop();

	private:
		int _argc;
		char **_argv;

        const uint32_t _window_width = 800;
        const uint32_t _window_height = 800;
        const char *_application_name = "Virtual Vista";

        GLFWWindow _window;
		Renderer *_renderer;
        Scene *_scene;

        void handleInput(float delta_time);
	};
}

#endif // VIRTUALVISTA_VIRTUALVISTAENGINE_H