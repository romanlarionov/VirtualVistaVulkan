
#include <stdexcept>
#include <chrono>

#include "VirtualVistaEngine.h"
#include "VulkanRenderer.h"
#include "InputManager.h"
#include "Settings.h"

namespace vv
{
    VirtualVistaEngine::VirtualVistaEngine(int argc, char **argv)
    : _argc(argc)
    , _argv(argv)
    {
    }


    VirtualVistaEngine::~VirtualVistaEngine()
    {
    }


    void VirtualVistaEngine::create()
    {
        _window.create(_window_width, _window_height, _application_name);

    	_renderer = new VulkanRenderer();
    	_renderer->create(&_window);
        _scene = _renderer->getScene();
    }


    void VirtualVistaEngine::shutDown()
    {
    	_renderer->shutDown();
    }


    Scene* VirtualVistaEngine::getScene() const
    {
        return _scene;
    }


    void VirtualVistaEngine::handleInput(float delta_time)
    {
        float move_speed = 4.0f * delta_time;
        float rotate_speed = 2.0f * delta_time;
        Camera *camera = _scene->getActiveCamera();
        if (InputManager::inst()->keyIsPressed(GLFW_KEY_W))
            camera->translate(move_speed * camera->getForwardDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_A))
            camera->translate(-move_speed * camera->getSidewaysDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_S))
            camera->translate(-move_speed * camera->getForwardDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_D))
            camera->translate(move_speed * camera->getSidewaysDirection());

        if (InputManager::inst()->keyIsPressed(GLFW_KEY_ESCAPE))
            _window.setShouldClose(true);

        double delta_x, delta_y;
        InputManager::inst()->getCursorGradient(delta_x, delta_y);
        camera->rotate(delta_x * rotate_speed, delta_y * rotate_speed);
    }


    void VirtualVistaEngine::beginMainLoop()
    {
        _renderer->recordCommandBuffers();

        auto last_time = glfwGetTime();

    	while (!_renderer->shouldStop())
    	{
            auto curr_time = glfwGetTime();
            float delta_time = curr_time - last_time;
            last_time = curr_time;

            _window.run();

            handleInput(delta_time);
    		_renderer->run(delta_time);
    	}
    }
}