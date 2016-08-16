
#include <iostream>
#include <stdexcept>

#include "VulkanRenderer.h"
#include "GLFWWindow.h"
#include "Settings.h"

namespace vv
{
	//////////////////////////////// Public
	VulkanRenderer::VulkanRenderer()
	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
	}

	void VulkanRenderer::init()
	{
		try {
			initWindow();
			createVulkanInstance();
		}
		catch (const std::runtime_error& e) {
			throw e;
		}
	}

	void VulkanRenderer::run()
	{
		// mainly polls events
		window_->run();

		// perform actual rendering calls
	}

	bool VulkanRenderer::shouldStop()
	{
		// todo: add other conditions
		return window_->shouldClose();
	}

	//////////////////////////////// Private
	void VulkanRenderer::initWindow()
	{
		if (Settings::inst()->getWindowType() == GLFW)
			window_ = new GLFWWindow;
		window_->init();
	}

	void VulkanRenderer::createVulkanInstance()
	{
		if (enable_validation_layers_ && !checkValidationLayerSupport())
			throw std::runtime_error("Validation layers requested are not available on this system.");

		// Instance Creation
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = Settings::inst()->getApplicationName().c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 3);
		app_info.pEngineName = Settings::inst()->getEngineName().c_str();
		app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		// Ensure required extensions are found.
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	
		if (!checkGLFWExtensionSupport(glfw_extension_count, glfw_extensions))
			throw std::runtime_error("Extensions requested, but are not available on this system.");

		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledExtensionCount = glfw_extension_count;
		instance_create_info.ppEnabledExtensionNames = glfw_extensions;
		instance_create_info.enabledLayerCount = 0;

		VV_CHECK_SUCCESS(vkCreateInstance(&instance_create_info, nullptr, &instance_), "Vulkan Error: failed to initialize instance");
	}

	/*
	 * Checks to see if the required 3rd party Vulkan extensions are available on the current system
	 * todo: maybe make more generailized by supporting any extension? (or wait til something like glew comes out)
	 */
	bool VulkanRenderer::checkGLFWExtensionSupport(uint32_t glfw_extension_count, const char** glfw_extensions)
	{
		uint32_t extension_count = 0;
		VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr), "vkEnumerateInstanceExtensionProperties failed. VulkanRenderer.cpp");
		std::vector<VkExtensionProperties> available_extensions(extension_count);
		VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data()), "vkEnumerateInstanceExtensionProperties failed. VulkanRenderer.cpp");

		// Linearally compare found extensions with requested ones.
		for (int i = 0; i < glfw_extension_count; ++i)
		{
			char* extension = const_cast<char*>(glfw_extensions[i]);
			bool extension_found = false;
			
			for (const auto& found_extension : available_extensions)
			{
				if (strcmp(extension, found_extension.extensionName) == 0)
				{
					extension_found = true;
					break;
				}
			}

			if (!extension_found) return false;
		}

		return true;
	}

	/* Checks to see if the validation layers that were requested are available on the current system
	 * FOR DEBUGGING PURPOSES ONLY 
	 */
	bool VulkanRenderer::checkValidationLayerSupport()
	{
		uint32_t layer_count = 0;
		VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, nullptr), "vkEnumerateInstanceLayerProperties failed. VulkanRenderer.cpp");
		std::vector<VkLayerProperties> available_layers(layer_count);
		VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()), "vkEnumerateInstanceLayerProperties failed. VulkanRenderer.cpp");

		// Linearally compare found layers with requested ones.
		for (const char* layer : validation_layers_)
		{
			bool layer_found = false;

			for (const auto& found_layer : available_layers)
			{
				if (strcmp(layer, found_layer.layerName) == 0)
				{
					layer_found = true;
					break;
				}
			}

			if (!layer_found) return false;
		}

		return true;
	}
}