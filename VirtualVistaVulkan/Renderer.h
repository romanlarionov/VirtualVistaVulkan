
#ifndef VIRTUALVISTA_RENDERER_H
#define VIRTUALVISTA_RENDERER_H

#include "GLFWWindow.h"

namespace vv 
{
    class Renderer
    {
    public:
		Renderer() {};
		~Renderer() {};

		virtual void init() {};
		virtual void run() {};
		virtual bool shouldStop() { return false; };
		virtual GLFWWindow* getWindow() { return nullptr; };
    };
}

#endif // VIRTUALVISTA_RENDERER_H