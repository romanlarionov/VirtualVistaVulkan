
#ifndef VIRTUALVISTA_GLFWWINDOW_H
#define VIRTUALVISTA_GLFWWINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vv 
{
    class GLFWWindow
    {
    public:
        GLFWWindow();
        ~GLFWWindow();

		void run();
		bool shouldClose();
		GLFWwindow* getHandle();

	private:
		GLFWwindow *window_; // glfw typedef
    };
}

#endif // VIRTUALVISTA_GLFWWINDOW_H