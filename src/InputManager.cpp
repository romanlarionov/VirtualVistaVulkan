
#include "InputManager.h"

namespace vv
{
    InputManager* InputManager::_instance = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////// Public
    InputManager* InputManager::inst()
    {
        if (!_instance)
            _instance = new InputManager;

        return _instance;
    }


    bool InputManager::keyIsPressed(int key) const
    {
        if ((key >= 0) && (key < GLFW_KEY_LAST))
            return _key_pressed_tracker[key];
        return false;
    }


    void InputManager::getCursorGradient(double &delta_x, double &delta_y)
    {
        delta_x = _delta_x;
        delta_y = _delta_y;
        _delta_x = 0.0;
        _delta_y = 0.0;
    }

    
    void InputManager::getCursorCoordinates(double &x, double &y) const
    {
        x = _curr_x;
        y = _curr_y;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////// Public
    InputManager::InputManager() :
        _curr_x(0.0),
        _curr_y(0.0),
        _delta_x(0.0),
        _delta_y(0.0),
        _first_input(true)
    {
        _key_pressed_tracker.resize(GLFW_KEY_LAST);
    }


    void InputManager::keyboardEventsCallback(GLFWwindow *window, int key, int scan_code, int action, int mods)
    {
        if ((key >= 0) && (key < GLFW_KEY_LAST))
        {
            if (action == GLFW_PRESS)
                _key_pressed_tracker[key] = true;
            else if (action == GLFW_RELEASE)
                _key_pressed_tracker[key] = false;
        }
    }


    void InputManager::mouseEventsCallback(GLFWwindow *window, double curr_x, double curr_y)
    {
        double temp_x = _curr_x;
        double temp_y = _curr_y;
        _curr_x = curr_x;
        _curr_y = curr_y;
        _delta_x = _curr_x - temp_x;
        _delta_y = _curr_y - temp_y;

        if (_first_input && _curr_x != 0 && _curr_y != 0)
        {
            _delta_x = 0;
            _delta_y = 0;
            _first_input = false;
        }
    }
}