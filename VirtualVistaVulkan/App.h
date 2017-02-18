
#ifndef VIRTUALVISTA_APP_H
#define VIRTUALVISTA_APP_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "VulkanRenderer.h"

namespace vv
{
	class App
	{
	public:
		App(int argc, char **argv);
		~App();

		void create();
		void shutDown();
		void mainLoop();

	private:
		int _argc;
		char **_argv;

		VulkanRenderer *_renderer;

	};
}

#endif // VIRTUALVISTA_APP_H