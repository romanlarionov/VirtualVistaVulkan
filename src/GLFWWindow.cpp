
#include <iostream>

#include "GLFWWindow.h"
#include "InputManager.h"
#include "Settings.h"
#include "Utils.h"

namespace vv
{
    void GLFWWindow::create(const int width, const int height, const char *application_name)
	{
		VV_ASSERT(glfwInit() != 0, "GLFW failed to init");
        window_width = width;
        window_height = height;
        application_name = application_name;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't use OpenGL
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(window_width, window_height, application_name, nullptr, nullptr);

		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
	}


	void GLFWWindow::createSurface(VkInstance instance)
	{
		VV_CHECK_SUCCESS(glfwCreateWindowSurface(instance, window, nullptr, &surface));
	}


	void GLFWWindow::shutDown(VkInstance instance)
	{
        surface_settings.clear();

		if (surface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(instance, surface, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}


	void GLFWWindow::run()
	{
		glfwPollEvents();
	}


    void GLFWWindow::setShouldClose(bool should_close)
    {
        glfwSetWindowShouldClose(window, should_close);
    }


	bool GLFWWindow::shouldClose()
	{
		return glfwWindowShouldClose(window) == 1;
	}


    void GLFWWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        InputManager::inst()->keyboardEventsCallback(window, key, scancode, action, mods);
    }


    void GLFWWindow::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        InputManager::inst()->mouseEventsCallback(window, xpos, ypos);
    }
}