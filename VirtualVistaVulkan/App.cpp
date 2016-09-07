
#include <stdexcept>

#include "App.h"
#include "VulkanRenderer.h"
#include "Settings.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	App::App(int argc, char **argv)
	{
		argc_ = argc;
		argv_ = argv;

		try
		{
			if (Settings::inst()->getRendererType() == VULKAN)
				renderer_ = new VulkanRenderer;

			renderer_->init();

		}
		catch (const std::runtime_error& e)
		{
			throw e;
		}
	}


	App::~App()
	{
	}


	void App::mainLoop()
	{
		// todo: add other conditionals for stopping execution
		while (!renderer_->shouldStop())
		{
			renderer_->run();
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private

}