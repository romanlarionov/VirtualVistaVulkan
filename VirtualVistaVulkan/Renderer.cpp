
#include <cstdlib>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "Renderer.h"
#include "Utils.h"

using namespace std;

namespace vv
{
    PFN_vkCreateDebugReportCallbackEXT  vkCreateDebugReportCallbackEXT = nullptr;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;

    Renderer::Renderer()
    {

#ifndef NDEBUG
        _setupDebug();
#endif

        _initInstance();

#ifndef NDEBUG
        _initDebug();
#endif

        _initDevice();
    }

    Renderer::~Renderer()
    {
        _device.destroy();
        _deinitDebug();
        _instance.destroy();
    }

    void Renderer::_initInstance()
    {
        /* setting up application specific info for Vulkan: */
        vk::ApplicationInfo application_info;
        application_info.apiVersion                     = VK_MAKE_VERSION(1, 0, 3);
        application_info.engineVersion                  = VK_MAKE_VERSION(0, 0, 1);
        application_info.pEngineName                    = "VirtualVista";

        vk::InstanceCreateInfo instance_create_info;
        instance_create_info.pApplicationInfo           = &application_info;
        instance_create_info.enabledLayerCount          = _instance_layers.size();
        instance_create_info.ppEnabledLayerNames        = _instance_layers.data();
        instance_create_info.enabledExtensionCount      = _instance_extensions.size();
        instance_create_info.ppEnabledExtensionNames    = _instance_extensions.data();
        instance_create_info.pNext                      = &_debug_callback_create_info;

        vk::createInstance(&instance_create_info, nullptr, &_instance);
    }

    void Renderer::_initDevice()
    {
        /* Get information about the actual gpu: */
        std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
        _gpu = physicalDevices[0];

        /*
         * Gather info on currently installed instance layers:
         * NOTE: Layers are used to debug certain aspects of the vulkan pipeline.
         */
        std::vector<vk::LayerProperties> layer_properties = vk::enumerateInstanceLayerProperties();

        /* Get information about which queue families are used by this device: */
        std::vector<vk::QueueFamilyProperties> queue_family_properties = _gpu.getQueueFamilyProperties();

        // consider making this async
        bool found = false;
        for (int i = 0; i < queue_family_properties.size() && !found; ++i)
        {
            // todo: check if this even works
            if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                found = true;
                _graphics_family_index = i;
            }
        }

        /* Set up info for device queue: */
        float queue_properties[]{1.0f};
        vk::DeviceQueueCreateInfo device_queue_create_info;
        device_queue_create_info.queueFamilyIndex   = _graphics_family_index;
        device_queue_create_info.queueCount         = 1;
        device_queue_create_info.pQueuePriorities   = queue_properties;

        /* Set up infor for device: */
        vk::DeviceCreateInfo device_create_info;
        device_create_info.queueCreateInfoCount     = 1; // number of processing queues used by this virtual device
        device_create_info.pQueueCreateInfos        = &device_queue_create_info; // specific info about this single queue

        _gpu.createDevice(&device_create_info, nullptr, &_device);
    }

    /* Setup for external extension callback for debugging */
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
        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
            stream << "INFORMATION: ";
        } if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            stream << "WARNING: ";
        } if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
            stream << "PERFORMANCE: ";
        } if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
            stream << "ERROR: ";
        } if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
            stream << "DEBUG: ";
        }

        stream << "@[" << layer_prefix << "]" << std::endl;
        stream << msg << std::endl;
        std::cout << stream.str() << std::endl;

#ifdef _WIN32
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            MessageBox(NULL, stream.str().c_str(), "VirtualVista Vulkan Error", 0);
#endif

        return false;
    }

    void Renderer::_setupDebug()
    {
        _debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        _debug_callback_create_info.pfnCallback = vulkanDebugCallback;
        _debug_callback_create_info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                           VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                           VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                           VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                           VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                           VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;

        _instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
        /*_instance_layers.push_back("VK_LAYER_LUNARG_draw_state");
        _instance_layers.push_back("VK_LAYER_LUNARG_image");
        _instance_layers.push_back("VK_LAYER_LUNARG_mem_tracker");
        _instance_layers.push_back("VK_LAYER_LUNARG_object_tracker");
        _instance_layers.push_back("VK_LAYER_LUNARG_param_checker");*/

        _instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    void Renderer::_initDebug()
    {
        vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)_instance.getProcAddr("vkCreateDebugReportCallbackEXT");
        vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)_instance.getProcAddr("vkDestroyDebugReportCallbackEXT");
        //VV_CHECK_ERROR(((nullptr == vkCreateDebugReportCallbackEXT) || (nullptr == vkDestroyDebugReportCallbackEXT)), "Cannot Find Debug Function Pointers");

        /* This extension doesn't play well with vkcpp, so I had to use the standard C api */
        vkCreateDebugReportCallbackEXT(_instance, &_debug_callback_create_info, nullptr, &_debug_report);
    }

    void Renderer::_deinitDebug()
    {
        vkDestroyDebugReportCallbackEXT(_instance, _debug_report, nullptr);
        _debug_report = nullptr;
    }
}