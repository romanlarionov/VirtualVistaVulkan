
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <functional>

#include "VulkanRenderer.h"
#include "GLFWWindow.h"
#include "Utils.h"
#include "Settings.h"

#ifdef _WIN32
#include <Windows.h>
#endif

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
			setupDebugCallback();
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
		switch (Settings::inst()->getWindowType())
		{
			case GLFW: window_ = new GLFWWindow;
					   break;
			default:   break;
		}
		window_->init();
	}

	void VulkanRenderer::createVulkanInstance()
	{
		if (enable_validation_layers_ && !checkValidationLayerSupport())
			throw std::runtime_error("Validation layers requested are not available on this system.");

		// Instance Creation
		std::string application_name = Settings::inst()->getApplicationName();
		std::string engine_name = Settings::inst()->getEngineName();

		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = application_name.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 3);
		app_info.pEngineName = engine_name.c_str();
		app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		// Ensure required extensions are found.
		if (!checkExtensionSupport())
			throw std::runtime_error("Extensions requested, but are not available on this system.");

		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &app_info;

		auto required_extensions = getRequiredExtensions();
		instance_create_info.enabledExtensionCount = required_extensions.size();
		instance_create_info.ppEnabledExtensionNames = required_extensions.data();

		if (enable_validation_layers_)
		{
			instance_create_info.enabledLayerCount = validation_layers_.size();
			instance_create_info.ppEnabledLayerNames = validation_layers_.data();
		}
		else
			instance_create_info.enabledLayerCount = 0;

		VV_CHECK_SUCCESS(vkCreateInstance(&instance_create_info, nullptr, &instance_), "Vulkan Error: failed to initialize instance");
		//VkResult res = vkCreateInstance(&instance_create_info, nullptr, &instance_);
	}
	
	/*
	 * Returns back a formatted list of all extensions used by the system.
	 */
	std::vector<const char*> VulkanRenderer::getRequiredExtensions()
	{
	    std::vector<const char*> extensions;
	    unsigned int glfwExtensionCount = 0;
	    const char** glfwExtensions;
	    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	    for (unsigned int i = 0; i < glfwExtensionCount; i++)
	        extensions.push_back(glfwExtensions[i]);

	    if (enable_validation_layers_)
	        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	    
	    return extensions;
	}

	/*
	 * Checks to see if the required 3rd party Vulkan extensions are available on the current system
	 * todo: maybe make more generailized by supporting any extension? (or wait til something like glew comes out)
	 */
	bool VulkanRenderer::checkExtensionSupport()
	{
		std::vector<const char*> required_extensions = getRequiredExtensions();

		uint32_t extension_count = 0;
		VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr), "vkEnumerateInstanceExtensionProperties failed. VulkanRenderer.cpp");
		std::vector<VkExtensionProperties> available_extensions(extension_count);
		VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data()), "vkEnumerateInstanceExtensionProperties failed. VulkanRenderer.cpp");

		// Linearally compare found extensions with requested ones.
		for (const auto& extension : required_extensions)
		{
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

	/* 
	 * Checks to see if the validation layers that were requested are available on the current system
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

	/*
	 * Callback that performs all printing of validation layer info.
	 */
    VKAPI_ATTR VkBool32 VKAPI_CALL
        vulkanDebugCallback(VkDebugReportFlagsEXT flags,
                            VkDebugReportObjectTypeEXT obj_type, // object that caused the error
                            uint64_t src_obj,
                            size_t location,
                            int32_t msg_code,
                            const char *layer_prefix,
                            const char *msg,
                            void *usr_data)
    {
        // todo: maybe add logging to files
        // todo: think of other things that might be useful to log

        std::ostringstream stream;
        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
            stream << "INFORMATION: ";
        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
            stream << "WARNING: ";
        if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
            stream << "PERFORMANCE: ";
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            stream << "ERROR: ";
        if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
            stream << "DEBUG: ";

        stream << "@[" << layer_prefix << "]" << std::endl;
        stream << msg << std::endl;
        std::cout << stream.str() << std::endl;

#ifdef _WIN32
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            MessageBox(NULL, stream.str().c_str(), "VirtualVista Vulkan Error", 0);
#endif

        return false;
    }

	VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
										  const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pCallback);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
	    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	    if (func != nullptr)
	        func(instance, callback, pAllocator);
	}

	/*
	 * Sets up all debug callbacks that handle accepting and printing validation layer reports.
	 */
	void VulkanRenderer::setupDebugCallback()
	{
		// Only execute if debugging to save in runtime execution processing.
		if (!enable_validation_layers_) return;

		VkDeleter<VkDebugReportCallbackEXT> debug_callback_ { instance_, destroyDebugReportCallbackEXT };
		VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
		debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_callback_create_info.pfnCallback = vulkanDebugCallback;
		debug_callback_create_info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;

		VV_CHECK_SUCCESS(createDebugReportCallbackEXT(instance_, &debug_callback_create_info, nullptr, &debug_callback_), "createDebugReportCallbackEXT failed. VulkanRenderer.cpp");
	}
}