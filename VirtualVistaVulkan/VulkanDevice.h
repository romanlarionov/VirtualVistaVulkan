
#ifndef VIRTUALVISTA_VULKANDEVICE_H
#define VIRTUALVISTA_VULKANDEVICE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include "GLFWWindow.h"
#include "Settings.h"
#include "Utils.h"

namespace vv
{
	struct VulkanSurfaceDetailsHandle
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		std::vector<VkSurfaceFormatKHR> available_surface_formats;
		std::vector<VkPresentModeKHR> available_surface_present_modes;
	};

	/* 
	 * todo: abstract more properties and features to consumable formats.
	 */
	struct VulkanDevice
	{
	public:
		// Settings and information related to the gpu itself. i.e. name, memory, whether there is VR support, etc.
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkDevice logical_device = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties physical_device_properties;
		VkPhysicalDeviceFeatures physical_device_features;
		VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
		std::vector<VkQueueFamilyProperties> queue_family_properties;

		VkQueue graphics_queue;

		const std::vector<const char*> used_validation_layers_ = { "VK_LAYER_LUNARG_standard_validation" };

		// General, abstracted, consumable information
		int graphics_family_index = -1;
		int compute_family_index = -1;
		int display_family_index = -1;

		VulkanDevice(VkPhysicalDevice device)
		{
			physical_device = device;
			VV_ASSERT(physical_device, "Vulkan Physical Device NULL");

			// Query and format all data related to this gpu.
			vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
			vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);
			vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

			uint32_t queue_family_properties_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);
			VV_ASSERT(queue_family_properties_count > 0, "vkGetPhysicalDeviceQueueFamilyProperties returned 0");
			queue_family_properties.resize(queue_family_properties_count);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, queue_family_properties.data());
		};

		~VulkanDevice()
		{
			if (logical_device != VK_NULL_HANDLE)
				vkDestroyDevice(logical_device, nullptr);
		};

		/*
		 * Returns if this physical device will be suitable for all compute and rendering purposes.
		 * todo: fill out more.
		 */
		bool isSuitable(VkSurfaceKHR surface, VulkanSurfaceDetailsHandle &surface_details)
		{
			if (surface == VK_NULL_HANDLE) return false;
			bool result = true;
			
			queryQueueFamilies();

			if (Settings::inst()->isGraphicsRequired() && graphics_family_index < 0)
				result = false;
			if (Settings::inst()->isComputeRequired() && compute_family_index < 0)
				result = false;
			if (Settings::inst()->isOnScreenRenderingRequired() && !querySwapChainSupport(surface, surface_details))
				result = false;
		
			return result;
		}

		void createLogicalDevice(bool swap_chain_support = true, VkQueueFlags queue_types = VK_QUEUE_GRAPHICS_BIT)
		{
			std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
			float default_queue_priority = 1.0f;

			if (queue_types & VK_QUEUE_GRAPHICS_BIT)
			{
				VkDeviceQueueCreateInfo queue_create_info = {};
				queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queue_create_info.queueFamilyIndex = graphics_family_index;
				queue_create_info.queueCount = 1;
				queue_create_info.pQueuePriorities = &default_queue_priority;
				device_queue_create_infos.push_back(queue_create_info);
			}

			// todo: adding the compute section causes a crash.
			if (queue_types & VK_QUEUE_COMPUTE_BIT)
			{
				VkDeviceQueueCreateInfo queue_create_info = {};
				queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queue_create_info.queueFamilyIndex = compute_family_index;
				queue_create_info.queueCount = 1;
				queue_create_info.pQueuePriorities = &default_queue_priority;
				device_queue_create_infos.push_back(queue_create_info);
			}

			// device extensions
			std::vector<const char*> device_extensions;
			if (swap_chain_support && checkDeviceExtensionSupport(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
				device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			VkDeviceCreateInfo device_create_info;
			device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			device_create_info.flags = 0;
			device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
			device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size());
			device_create_info.pEnabledFeatures = &physical_device_features;

			if (!device_extensions.empty())
			{
				device_create_info.enabledExtensionCount = device_extensions.size();
				device_create_info.ppEnabledExtensionNames = device_extensions.data();
			}
			else
				device_create_info.enabledExtensionCount = 0;

#ifdef _DEBUG
			// todo: currently using the same layers for the device that I do for the instance level. might
			// need to diverge here (but it's my understanding that device layers are becoming depricated).
			device_create_info.enabledLayerCount = used_validation_layers_.size();
			device_create_info.ppEnabledLayerNames = used_validation_layers_.data();
#else
			device_create_info.enabledLayerCount = 0;
#endif

			VV_CHECK_SUCCESS(vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device));

			// sets up graphics queue handle.
			// todo: maybe move from here/investigate what command pools are
			vkGetDeviceQueue(logical_device, graphics_family_index, 0, &graphics_queue);
		}

	private:
		/*
		 * Checks to see if this gpu has swap chain support (creating queues of rendered frames to pass to a window system)
		 */
		bool querySwapChainSupport(VkSurfaceKHR surface, VulkanSurfaceDetailsHandle &surface_details_handle)
		{
			if (surface == nullptr) return false;

			int i = 0;
			for (auto family : queue_family_properties)
			{
				VkBool32 display_supported = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &display_supported);
				if (display_supported)
					display_family_index = i;
				i++;
			}

			if (display_family_index < 0) return false;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_details_handle.surface_capabilities);

			uint32_t surface_format_count = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr);
			if (surface_format_count == 0) return false;
			
			surface_details_handle.available_surface_formats.resize(surface_format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_details_handle.available_surface_formats.data());

			uint32_t surface_present_mode_count = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &surface_present_mode_count, nullptr);
			if (surface_present_mode_count == 0) return false;

			surface_details_handle.available_surface_present_modes.resize(surface_present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &surface_present_mode_count, surface_details_handle.available_surface_present_modes.data());
			
			return true;
		}

		/*
		 * Parses Vulkan acquired property data for this physical device and stores any important info.
		 * todo: store more data that might prove valuable.
		 */
		void queryQueueFamilies()
		{
			// Search for queue families
			int i = 0;
			for (auto family : queue_family_properties)
			{
				if (family.queueCount > 0)
				{
					if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
						graphics_family_index = i;
					if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
						compute_family_index = i;
				}
				i++;
			}
		}

		/*
		 * Checks if the requested device level extension is presently available.
		 */
		bool checkDeviceExtensionSupport(const char* extension)
		{
			uint32_t extension_count = 0;
			VV_CHECK_SUCCESS(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr));
			std::vector<VkExtensionProperties> available_extensions(extension_count);
			VV_CHECK_SUCCESS(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data()));

			for (auto& e : available_extensions)
				if (strcmp(e.extensionName, extension) == 0)
					return true;

			return false;
		}
	};
}

#endif // VIRTUALVISTA_VULKANDEVICE_H