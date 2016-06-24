
#ifndef VIRTUALVISTA_RENDERER_H
#define VIRTUALVISTA_RENDERER_H

#include <vector>
//#include <vulkan/vulkan.h>
#include "vk_cpp.hpp"

namespace vv 
{
    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

    private:
        ////////////////////////////////////////////////////////////////////////////////////
        /* Instance from which everything is based off of */
        //VkInstance _instance = nullptr;
        vk::Instance _instance = nullptr;

        /*
         * A handle to the physical gpu being used for computation
         * NOTE: can support multiple gpus if available.
         */
        //VkPhysicalDevice _gpu = nullptr;
        vk::PhysicalDevice _gpu = nullptr;

        /* Stores information about the gpu that is being use for computation */
        //VkPhysicalDeviceProperties _physical_device_properties = {};
        vk::PhysicalDeviceProperties _physical_device_properties = {};

        /*
         * A virtual device that can perform async computations.
         * Maps: 1 VkPhysicalDevice -> many VkDevice -> many VkQueueFamily
         * NOTE: can be extended to have multiple VkDevices through multithreading.
         */
        //VkDevice _device = nullptr;
        vk::Device _device = nullptr;

        /* Stores the location */
        int _graphics_family_index;

        /* Hold information pertaining to debug tools for the application instance */
        std::vector<const char*> _instance_layers;
        std::vector<const char*> _instance_extensions;

        /* Callback handle for debug logging */
        VkDebugReportCallbackEXT _debug_report = nullptr;
        VkDebugReportCallbackCreateInfoEXT _debug_callback_create_info = {};

        ////////////////////////////////////////////////////////////////////////////////////
        void _initInstance();
        void _initDevice();

        void _setupDebug();
        void _initDebug();
        void _deinitDebug();
    };
}

#endif // VIRTUALVISTA_RENDERER_H