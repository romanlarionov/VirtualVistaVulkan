
#include <iostream>
#include <stdexcept>

#include "VulkanRenderer.h"
#include "GLFWWindow.h"
#include "Settings.h"
#include "Utils.h"

namespace vv
{
	VulkanRenderer::VulkanRenderer()
	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
	}

	void VulkanRenderer::init()
	{
		try {
			initWindow();
		}
		catch (const std::runtime_error& e) {
			throw e;
		}
	}

	void VulkanRenderer::run()
	{
		// mainly polls events
		window_->run();

		// perform actual rendering calls
	}

	bool VulkanRenderer::shouldStop()
	{
		// todo: add other conditions
		bool temp = window_->shouldClose();
		return temp;
	}

	void VulkanRenderer::initWindow()
	{
		if (Settings::inst()->getWindow() == GLFW)
			window_ = new GLFWWindow;
		window_->init();
	}

	Window* VulkanRenderer::getWindow()
	{
		return window_;
	}
}