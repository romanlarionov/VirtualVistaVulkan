
#include <iostream>

#include "GLFWWindow.h";
#include "Settings.h";

namespace vv
{
	GLFWWindow::GLFWWindow()
	{
	}

	GLFWWindow::~GLFWWindow()
	{
	}

	void GLFWWindow::init()
	{
		if (!glfwInit())
		{
			throw std::runtime_error("GLFW failed to init"); // todo: maybe make more elegant
		}

		// Don't use OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // todo: replace later (for tutorial purposes only)
		window_ = glfwCreateWindow(Settings::inst()->getWidth(), Settings::inst()->getHeight(), Settings::inst()->getName().c_str(), nullptr, nullptr); // todo: replace hardcoded values
	}

	void GLFWWindow::run()
	{
		glfwPollEvents();
		// fill the rest in here
	}

	bool GLFWWindow::shouldClose()
	{
		return glfwWindowShouldClose(window_);
	}
}