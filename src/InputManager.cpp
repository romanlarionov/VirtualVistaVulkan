
#include "InputManager.h"

namespace vv
{
    InputManager* InputManager::m_instance = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////// Public
    InputManager* InputManager::inst()
    {
        if (!m_instance)
            m_instance = new InputManager;

        return m_instance;
    }


    bool InputManager::keyIsPressed(int key) const
    {
        if ((key >= 0) && (key < GLFW_KEY_LAST))
            return m_key_pressed_tracker[key];
        return false;
    }


    void InputManager::getCursorGradient(double &delta_x, double &delta_y)
    {
        delta_x = m_delta_x;
        delta_y = m_delta_y;
        m_delta_x = 0.0;
        m_delta_y = 0.0;
    }

    
    void InputManager::getCursorCoordinates(double &x, double &y) const
    {
        x = m_curr_x;
        y = m_curr_y;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////// Public
    InputManager::InputManager() :
        m_curr_x(0.0),
        m_curr_y(0.0),
        m_delta_x(0.0),
        m_delta_y(0.0),
        m_first_input(true)
    {
        m_key_pressed_tracker.resize(GLFW_KEY_LAST);
    }


    void InputManager::keyboardEventsCallback(GLFWwindow *window, int key, int scan_code, int action, int mods)
    {
        if ((key >= 0) && (key < GLFW_KEY_LAST))
        {
            if (action == GLFW_PRESS)
                m_key_pressed_tracker[key] = true;
            else if (action == GLFW_RELEASE)
                m_key_pressed_tracker[key] = false;
        }
    }


    void InputManager::mouseEventsCallback(GLFWwindow *window, double curr_x, double curr_y)
    {
        double temp_x = m_curr_x;
        double temp_y = m_curr_y;
        m_curr_x = curr_x;
        m_curr_y = curr_y;
        m_delta_x = m_curr_x - temp_x;
        m_delta_y = m_curr_y - temp_y;

        if (m_first_input && m_curr_x != 0 && m_curr_y != 0)
        {
            m_delta_x = 0;
            m_delta_y = 0;
            m_first_input = false;
        }
    }
}