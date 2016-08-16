
#ifndef VIRTUALVISTA_VULKANRENDERER_H
#define VIRTUALVISTA_VULKANRENDERER_H

#include <vector>

#include "Renderer.h"
#include "Utils.h"

template <typename T> class VkDeleter
	{
	public:
		VkDeleter() : VkDeleter([](T _) {})
		{
		}

		VkDeleter(std::function<void(T, VkAllocationCallbacks*)> deletef)
		{
			this->deleter = [=](T obj) { deletef(obj, nullptr); };
		}

		VkDeleter(const VkDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef)
		{
			this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
		}

		VkDeleter(const VkDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef)
		{
			this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
		}

		~VkDeleter()
		{
			cleanup();
		}

		T* operator &()
		{
			cleanup();
			return &object;
		}

		operator T() const
		{
			return object;
		}

	private:
		T object{ VK_NULL_HANDLE };
		std::function<void(T)> deleter;

		void cleanup() {
			if (object != VK_NULL_HANDLE) {
				deleter(object);
			}
			object = VK_NULL_HANDLE;
		}
	};

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
#ifdef _DEBUG
		const bool enable_validation_layers_ = true;
#else
		const bool enable_validation_layers_ = false;
#endif

		Window *window_;
		VkDeleter<VkInstance> instance_ {vkDestroyInstance};
		const std::vector<const char*> validation_layers_ = { "VK_LAYER_LUNARG_standard_validation" };

		void initWindow();
		void createVulkanInstance();
		void setupDebugCallback();

		std::vector<const char*> getRequiredExtensions();
		void createVulkanPhysicalDevice();
		bool checkExtensionSupport();
		bool checkValidationLayerSupport();
	};
}

#endif // VIRTUALVISTA_VULKANRENDERER_H