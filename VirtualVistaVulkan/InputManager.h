
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

        bool keyIsPressed(int key);
        void getMouseValues(double &x, double &y);

    private:
        static InputManager *_instance;
        double _curr_x;
        double _curr_y;
        std::vector<bool> _key_pressed_tracker;

        InputManager();
        InputManager(InputManager const&);
        InputManager& operator=(InputManager const&);

        // only used in one place, don't need to be seen by anything other than GLFWWindow
        void keyboardEventsCallback(GLFWwindow* window, int key, int scan_code, int action, int mods);
        void mouseEventsCallback(GLFWwindow *window, double curr_x, double curr_y);
    };
}

#endif // VIRTUALVISTA_INPUTMANAGER_H