
#ifndef VIRTUALVISTA_GLFWWINDOW_H
#define VIRTUALVISTA_GLFWWINDOW_H

// todo: check if this is the best place to put this
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Window.h"

namespace vv 
{
    class GLFWWindow : public Window 
    {
    public:
        GLFWWindow();
        ~GLFWWindow();

		void init();
		void run();
		bool shouldClose();

    private:
		GLFWwindow *window_; // glfw typedef

    };
}

#endif // VIRTUALVISTA_GLFWWINDOW_H