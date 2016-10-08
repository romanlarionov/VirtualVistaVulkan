
#include <iostream>

#include "GLFWWindow.h";
#include "Settings.h";
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
		VV_ASSERT(glfwInit(), "GLFW failed to init");

		// Don't use OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // todo: replace later (for tutorial purposes only)
		std::string application_name = Settings::inst()->getApplicationName();
		window = glfwCreateWindow(Settings::inst()->getWindowWidth(),
								  Settings::inst()->getWindowHeight(),
								  application_name.c_str(),
								  nullptr, nullptr);

		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
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
		// fill the rest in here
	}


	bool GLFWWindow::shouldClose()
	{
		return glfwWindowShouldClose(window);
	}


	void GLFWWindow::addSurfaceDetails(VulkanDevice *device, VulkanSurfaceDetailsHandle details)
	{
		
	}
	///////////////////////////////////////////////////////////////////////////////////////////// Private

}