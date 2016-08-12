
#ifndef VIRTUALVISTA_APP_H
#define VIRTUALVISTA_APP_H

#include "Renderer.h"

// todo: consider making this singleton
namespace vv
{
	class App
	{
	public:
		App();
		~App();

		void init();
		void mainLoop();

	private:
		Renderer *renderer_;

	};
}

#endif // VIRTUALVISTA_APP_H