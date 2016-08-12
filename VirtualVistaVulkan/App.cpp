
#include <stdexcept>

#include "App.h";
#include "VulkanRenderer.h"
#include "Settings.h"

namespace vv
{
	App::App()
	{
	}

	App::~App()
	{
	}

	void App::init()
	{
		if (Settings::inst()->getRenderer() == VULKAN)
			renderer_ = new VulkanRenderer;

		try {
			renderer_->init();
		} catch (const std::runtime_error& e) {
			throw e;
		}
	}

	void App::mainLoop()
	{
		// todo: add other conditionals for stopping execution
		while (!renderer_->shouldStop())
		{
			renderer_->run();
		}
	}
}