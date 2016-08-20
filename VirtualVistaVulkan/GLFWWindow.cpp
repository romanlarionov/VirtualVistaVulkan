
#include <iostream>

#include "GLFWWindow.h";
#include "Settings.h";

namespace vv
{
	GLFWWindow::GLFWWindow()
	{
		if (!glfwInit())
			throw std::runtime_error("GLFW failed to init");

		// Don't use OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // todo: replace later (for tutorial purposes only)
		std::string application_name = Settings::inst()->getApplicationName();
		window_ = glfwCreateWindow(Settings::inst()->getWindowWidth(),
								   Settings::inst()->getWindowHeight(),
								   application_name.c_str(),
								   nullptr, nullptr);
	}

	GLFWWindow::~GLFWWindow()
	{
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

	GLFWwindow* GLFWWindow::getHandle()
	{
		return window_;
	}
}