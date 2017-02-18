
#include <stdexcept>

#include "App.h"
#include "VulkanRenderer.h"
#include "Settings.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	App::App(int argc, char **argv) :
		_argc(argc),
		_argv(argv)
	{
	}


	App::~App()
	{
	}


	void App::create()
	{
		try
		{
			_renderer = new VulkanRenderer();
			_renderer->create();
		}
		catch (const std::runtime_error& e)
		{
			throw e;
		}
	}


	void App::shutDown()
	{
		_renderer->shutDown();
	}


	void App::mainLoop()
	{
		// todo: add other conditionals for stopping execution
		while (!_renderer->shouldStop())
		{
			_renderer->run();
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private

}