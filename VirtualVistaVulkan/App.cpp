
#include <stdexcept>
#include <chrono>

#include "App.h"
#include "VulkanRenderer.h"
#include "InputManager.h"
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
            _scene = _renderer->getScene();
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


    Scene* App::getScene() const
    {
        return _scene;
    }


	void App::beginMainLoop(std::function<void(Scene*, float)> input_handler)
	{
        _renderer->recordCommandBuffers();

        auto last_time = glfwGetTime();

		while (!_renderer->shouldStop())
		{
            auto curr_time = glfwGetTime();
            float delta_time = curr_time - last_time;
            last_time = curr_time;

            input_handler(_scene, delta_time);
			_renderer->run(delta_time);
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private

}