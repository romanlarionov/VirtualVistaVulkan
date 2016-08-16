
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>

#include "Renderer.h"
#include "Utils.h"

namespace vv
{
	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void init();
		void run();
		bool shouldStop();

	private:
		Window *window_;
		VDeleter<VkInstance> instance_ {vkDestroyInstance};

#ifdef NDEBUG
		const bool enable_validation_layers_ = false;
#else
		const bool enable_validation_layers_ = true;
#endif

		const std::vector<const char*> validation_layers_ = { "VK_LAYER_LUNARG_standard_validation" };

		void initWindow();
		void createVulkanInstance();
		void createVulkanPhysicalDevice();
		bool checkGLFWExtensionSupport(uint32_t glfw_extension_count, const char** glfw_extensions);
		bool checkValidationLayerSupport();

	};
}

#endif // VIRTUALVISTA_VULKANRENDERER_H