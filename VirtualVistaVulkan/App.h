
#ifndef VIRTUALVISTA_APP_H
#define VIRTUALVISTA_APP_H

#include "Renderer.h"

namespace vv
{
	class App
	{
	public:
		App(int argc, char **argv);
		~App();

		void mainLoop();

	private:
		int argc_;
		char **argv_;

		Renderer *renderer_;

	};
}

#endif // VIRTUALVISTA_APP_H