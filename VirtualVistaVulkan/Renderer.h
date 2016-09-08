
#ifndef VIRTUALVISTA_RENDERER_H
#define VIRTUALVISTA_RENDERER_H

#include "GLFWWindow.h"

namespace vv 
{
    class Renderer
    {
    public:
		Renderer() {};
		virtual ~Renderer() {};

		virtual void init() = 0;
		virtual void shutDown() = 0;
		virtual void run() = 0;
		virtual bool shouldStop() { return false; };
		virtual GLFWWindow* getWindow() { return nullptr; };
    };
}

#endif // VIRTUALVISTA_RENDERER_H