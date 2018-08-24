
#ifndef VIRTUALVISTA_VIRTUALVISTAENGINE_H
#define VIRTUALVISTA_VIRTUALVISTAENGINE_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "DeferredRenderer.h"
#include "GLFWWindow.h"
#include "Scene.h"

namespace vv
{
	class VirtualVistaEngine
	{
	public:
		VirtualVistaEngine() = default;
		~VirtualVistaEngine() = default;

        /*
         * Initializes all essential components required by the engine.
         */
		void create(int argc, char **argv);

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
        int m_argc;
        char **m_argv;

        const uint32_t m_window_width  = 1920;
        const uint32_t m_window_height = 1080;
        const char *m_application_name = "Virtual Vista";

        float m_move_speed = 4.0f;

#ifdef _DEBUG
        float m_rotate_speed = 0.2f;
#else
        float m_rotate_speed = 2.0f;
#endif

        GLFWWindow m_window;
        DeferredRenderer *m_renderer;
        Scene *m_scene;

        void handleInput(float delta_time);
	};
}

#endif // VIRTUALVISTA_VIRTUALVISTAENGINE_H