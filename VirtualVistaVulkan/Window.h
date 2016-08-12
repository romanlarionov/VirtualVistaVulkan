
#ifndef VIRTUALVISTA_WINDOW_H
#define VIRTUALVISTA_WINDOW_H

namespace vv 
{
    class Window 
    {
    public:
		Window() {};
		~Window() {};

		virtual void init() {};
		virtual void run() {};
		virtual bool shouldClose() { return true; };
    };
}

#endif // VIRTUALVISTA_WINDOW_H