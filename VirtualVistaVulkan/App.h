
#ifndef VIRTUALVISTA_APP_H
#define VIRTUALVISTA_APP_H

#include "VulkanRenderer.h"

namespace vv
{
	class App
	{
	public:
		App(int argc, char **argv);
		~App();

		void init();
		void shutDown();
		void mainLoop();

	private:
		int argc_;
		char **argv_;

		VulkanRenderer *renderer_;

	};
}

#endif // VIRTUALVISTA_APP_H