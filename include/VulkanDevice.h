
#ifndef VIRTUALVISTA_VULKANDEVICE_H
#define VIRTUALVISTA_VULKANDEVICE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_map>

#include "GLFWWindow.h"
#include "Settings.h"
#include "Utils.h"

namespace vv
{
    class VulkanDevice
    {
    public:
    	// Settings and information related to the GPU itself. i.e. name, memory, whether there is VR support, etc.
    	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    	VkDevice logical_device = VK_NULL_HANDLE;
    	VkPhysicalDeviceProperties physical_device_properties;
    	VkPhysicalDeviceFeatures physical_device_features;
    	VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    	std::vector<VkQueueFamilyProperties> queue_family_properties;

    	VkQueue graphics_queue = VK_NULL_HANDLE;
    	VkQueue compute_queue  = VK_NULL_HANDLE;
    	VkQueue transfer_queue = VK_NULL_HANDLE;

    	// General, abstracted, consumable information
    	int32_t graphics_family_index = -1;
    	int32_t compute_family_index  = -1;
    	int32_t transfer_family_index = -1;
    	int32_t display_family_index  = -1;

    	std::unordered_map<std::string, VkCommandPool> command_pools;

    	VulkanDevice() = default;
    	~VulkanDevice() = default;

    	/*
    	 * Creates all initial Vulkan internals.
    	 */
    	void create(VkPhysicalDevice device);

    	/*
    	 * Deletes all Vulkan internals.
    	 */
    	void shutDown();

    	/*
    	 * Returns if this physical device supports a graphics command queue.
    	 */
    	bool hasGraphicsQueue() const { return graphics_family_index >= 0; }

        /*
    	 * Returns if this physical device supports a compute command queue.
    	 */
    	bool hasComputeQueue() const { return compute_family_index >= 0; }

        /*
    	 * Returns if this physical device supports a transfer command queue.
    	 */
    	bool hasTransferQueue() const { return transfer_family_index >= 0; }

        /*
         * Creates the logical abstraction Vulkan needs to perform all of its functionality.
         *
         * todo: this takes 1 second to complete... why?
         */
    	void createLogicalDevice(bool swap_chain_support = true, VkQueueFlags queue_types = VK_QUEUE_GRAPHICS_BIT);

    	/*
    	 * Creates a Vulkan pool that stores GPU operations such as memory transfers, draw calls, and compute calls.
    	 */
    	void createCommandPool(std::string name, uint32_t queue_index, VkCommandPoolCreateFlags create_flags);

    	/*
    	 * Finds the index for the appropriate supported memory type for the given physical device.
    	 */
    	uint32_t findMemoryTypeIndex(uint32_t filter_type, VkMemoryPropertyFlags memory_property_flags);

    	/*
    	 * Checks to see if this GPU has swap chain support (creating queues of rendered frames to pass to a window system)
    	 */
	    VulkanSurfaceDetailsHandle querySwapChainSupport(VkSurfaceKHR surface);

    private:

    	/*
    	 * Parses Vulkan acquired property data for this physical device and stores any important info.
         *
    	 * todo: store more data that might prove valuable.
    	 */
    	void queryQueueFamilies();

    	/*
    	 * Checks if the requested device level extension is presently available.
    	 */
    	bool checkDeviceExtensionSupport(const char* extension);
    };
}

#endif // VIRTUALVISTA_VULKANDEVICE_H