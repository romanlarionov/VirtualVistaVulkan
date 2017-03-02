
#include <iostream>

#include "GLFWWindow.h"
#include "InputManager.h"
#include "Settings.h"
#include "Utils.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	GLFWWindow::GLFWWindow()
	{
	}


	GLFWWindow::~GLFWWindow()
	{
	}


    
	void GLFWWindow::create()
	{
		VV_ASSERT(glfwInit() != 0, "GLFW failed to init");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't use OpenGL

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		std::string application_name = Settings::inst()->getApplicationName();
		window = glfwCreateWindow(Settings::inst()->getWindowWidth(),
								  Settings::inst()->getWindowHeight(),
								  application_name.c_str(),
								  nullptr, nullptr);

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
		if (surface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(instance, surface, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void GLFWWindow::run()
	{
		glfwPollEvents();
	}


	bool GLFWWindow::shouldClose()
	{
		return glfwWindowShouldClose(window) == 1;
	}


	void GLFWWindow::addSurfaceDetails(VulkanDevice *device, VulkanSurfaceDetailsHandle details)
	{
		
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
    void GLFWWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        InputManager::inst()->keyboardEventsCallback(window, key, scancode, action, mods);
    }


    void GLFWWindow::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        InputManager::inst()->mouseEventsCallback(window, xpos, ypos);
    }
}