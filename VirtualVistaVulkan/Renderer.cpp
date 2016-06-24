
#include <cstdlib>
#include <iostream>
#include <vector>

#include "Renderer.h"
#include "Utils.h"

using namespace std;

namespace vv
{
    Renderer::Renderer()
    {
        _initInstance();
        _initDevice();
    }

    Renderer::~Renderer()
    {
        _deinitDevice();
        _deinitInstance();
    }

    void Renderer::_initInstance()
    {
        // setting up application specific info for Vulkan
        VkApplicationInfo application_info = {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.apiVersion = VK_MAKE_VERSION(1, 0, 3);
        application_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        application_info.pEngineName = "VirtualVista";

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;

        auto err = vkCreateInstance(&instance_create_info, nullptr, &_instance);
        VV_CHECK_ERROR(err, "Create Instance Failed");
    }

    void Renderer::_deinitInstance()
    {
        vkDestroyInstance(_instance, nullptr);
        _instance = nullptr;
    }

    void Renderer::_initDevice()
    {
        {
            /* Gets information about the actual gpu */
            uint32_t gpu_count = 0;
            vkEnumeratePhysicalDevices(_instance, &gpu_count, nullptr);
            std::vector<VkPhysicalDevice> physical_gpus(gpu_count);
            vkEnumeratePhysicalDevices(_instance, &gpu_count, physical_gpus.data());
            _gpu = physical_gpus[0];
            vkGetPhysicalDeviceProperties(_gpu, &_physical_device_properties);
        }
        {
            /* Gets information about which queue families are used by this device */
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &queue_family_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &queue_family_count, queue_family_properties.data());

            /* Look through all physical device queue family properties and try to find a graphics queue */
            bool found = false;
            for (uint32_t i = 0; i < queue_family_count && !found; ++i)
            {
                if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    found = true;
                    _graphics_family_index = i;
                }
            }

            if (!found)
            {
                std::cerr << "Vulkan ERROR: Queue Family Supporting Graphics Not Found\n";
                std::exit(-1); // todo: do something a little more elegant than exit
            }
        }

        float queue_properties[]{1.0f};

        VkDeviceQueueCreateInfo device_queue_create_info = {};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.queueFamilyIndex = _graphics_family_index;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.pQueuePriorities = queue_properties;

        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = 1; // number of processing queues used by this virtual device
        device_create_info.pQueueCreateInfos = &device_queue_create_info; // specific info about this single queue

        auto err = vkCreateDevice(_gpu, &device_create_info, nullptr, &_device);
        VV_CHECK_ERROR(err, "Create Device Failed");
    }

    void Renderer::_deinitDevice()
    {
        vkDestroyDevice(_device, nullptr);
        _device = nullptr;
    }




}