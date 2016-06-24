
#ifndef VIRTUALVISTA_RENDERER_H
#define VIRTUALVISTA_RENDERER_H

#include <vulkan/vulkan.h>

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
        VkInstance _instance = nullptr;

        /*
         * A handle to the physical gpu being used for computation
         * NOTE: can support multiple gpus if available.
         */
        VkPhysicalDevice _gpu = nullptr;

        /* Stores information about the gpu that is being use for computation */
        VkPhysicalDeviceProperties _physical_device_properties = {};

        /*
         * A virtual device that can perform async computations.
         * Maps: 1 VkPhysicalDevice -> many VkDevice -> many VkQueueFamily
         * NOTE: can be extended to have multiple VkDevices through multithreading.
         */
        VkDevice _device = nullptr;

        /* Stores the location */
        int _graphics_family_index;

        ////////////////////////////////////////////////////////////////////////////////////
        void _initInstance();
        void _initDevice();

        void _deinitInstance();
        void _deinitDevice();
    };
}

#endif // VIRTUALVISTA_RENDERER_H