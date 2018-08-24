
#ifndef VIRTUALVISTA_INPUTMANAGER_H
#define VIRTUALVISTA_INPUTMANAGER_H

#include <vector>

#include <GLFW/glfw3.h>

namespace vv
{
    class InputManager
    {
        friend class GLFWWindow;

    public:
        static InputManager* inst();

        bool keyIsPressed(int key) const;
        void getCursorGradient(double &delta_x, double &delta_y);
        void getCursorCoordinates(double &x, double &y) const;

    private:
        static InputManager *m_instance;
        double m_curr_x;
        double m_curr_y;
        double m_delta_x;
        double m_delta_y;
        bool m_first_input;
        std::vector<bool> m_key_pressed_tracker;

        InputManager();
        InputManager(InputManager const&);
        InputManager& operator=(InputManager const&);

        // only used in one place, don't need to be seen by anything other than GLFWWindow
        void keyboardEventsCallback(GLFWwindow* window, int key, int scan_code, int action, int mods);
        void mouseEventsCallback(GLFWwindow *window, double curr_x, double curr_y);
    };
}

#endif // VIRTUALVISTA_INPUTMANAGER_H