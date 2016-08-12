
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include "Renderer.h"

namespace vv
{
	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void init();
		void run();
		Window* getWindow();
		bool shouldStop();

	private:
		Window *window_;
		bool shouldStop_;

		void initWindow();
	};
}

#endif // VIRTUALVISTA_VULKANRENDERER_H