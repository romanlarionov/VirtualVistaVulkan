
#ifndef VIRTUALVISTA_VULKANRENDERERHELPER_H
#define VIRTUALVISTA_VULKANRENDERERHELPER_H

#include <vector>
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
	struct VulkanPhysicalDeviceHandle
	{
	public:
		// Settings and information related to the gpu itself. i.e. name, memory, whether there is VR support, etc.
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties physical_device_properties;
		VkPhysicalDeviceFeatures physical_device_features;

		// General, abstracted, consumable information
		int graphics_family_index = -1;
		int compute_family_index = -1;
		int display_family_index = -1;

		const std::vector<const char*> used_device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VulkanPhysicalDeviceHandle(VkPhysicalDevice device)
		{
			physical_device = device;

			// Query and format all data related to this gpu.
			vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
			vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);
		};

		~VulkanPhysicalDeviceHandle() {};

		/*
		 * Returns if this physical device will be suitable for all compute and rendering purposes.
		 * todo: fill out more.
		 */
		bool isSuitable(VkSurfaceKHR surface, VulkanSurfaceDetailsHandle &surface_details)
		{
			if (physical_device == VK_NULL_HANDLE || surface == VK_NULL_HANDLE || !checkDeviceExtensionSupport()) return false;
			bool result = true;

			queryQueueFamilies(surface);
			surface_details = querySwapChainSupport(surface);

			if (Settings::inst()->isGraphicsRequired() && (graphics_family_index < 0))
				result = false;
			if (Settings::inst()->isComputeRequired() && (compute_family_index < 0))
				result = false;
			if (Settings::inst()->isOnScreenRenderingRequired() && (display_family_index < 0) && surface_details.available_surface_present_modes.empty())
				result = false;
		
			return result;
		}

	private:
		/*
		 * Checks to see if this gpu has swap chain support (creating queues of rendered frames to pass to a window system)
		 */
		VulkanSurfaceDetailsHandle querySwapChainSupport(VkSurfaceKHR surface)
		{
			VulkanSurfaceDetailsHandle surface_details_handle = {};

			if (physical_device != VK_NULL_HANDLE && surface != VK_NULL_HANDLE)
			{
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_details_handle.surface_capabilities);

				uint32_t surface_format_count = 0;
				vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr);

				if (surface_format_count != 0)
				{
					surface_details_handle.available_surface_formats.resize(surface_format_count);
					vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_details_handle.available_surface_formats.data());
				}

				uint32_t surface_present_mode_count = 0;
				vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &surface_present_mode_count, nullptr);

				if (surface_present_mode_count != 0)
				{
					surface_details_handle.available_surface_present_modes.resize(surface_present_mode_count);
					vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &surface_present_mode_count, surface_details_handle.available_surface_present_modes.data());
				}
			}
			
			return surface_details_handle;
		}

		/*
		 * Parses Vulkan acquired property data for this physical device and stores any important info.
		 * todo: store more data that might prove valuable.
		 */
		void queryQueueFamilies(VkSurfaceKHR surface)
		{
			if (physical_device == VK_NULL_HANDLE) return;

			uint32_t queue_family_properties_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);

			std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_properties_count);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, queue_family_properties.data());

			// Search for queue families
			int i = 0;
			for (auto family : queue_family_properties)
			{
				VkBool32 display_supported = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &display_supported);
				if (family.queueCount > 0)
				{
					if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
						graphics_family_index = i;
					if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
						compute_family_index = i;
					if (display_supported)
						display_family_index = i;
				}
				i++;
			}
		}

		/*
		 * Checks if the required device level extensions are present.
		 * todo: might be better to pass a list of extensions as a parameter to avoid false positives between other aspects of the device.
		 */
		bool checkDeviceExtensionSupport()
		{
			if (physical_device == VK_NULL_HANDLE) return false;

			uint32_t extension_count = 0;
			VV_CHECK_SUCCESS(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr), "vkEnumerateDeviceExtensionProperties failed. VulkanRenderer.cpp");
			std::vector<VkExtensionProperties> available_extensions(extension_count);
			VV_CHECK_SUCCESS(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data()), "vkEnumerateDeviceExtensionProperties failed. VulkanRenderer.cpp");

			// Compare found extensions with requested ones.
			for (const auto& extension : used_device_extensions)
			{
				bool extension_found = false;
				for (const auto& found_extension : available_extensions)
					if (strcmp(extension, found_extension.extensionName) == 0)
					{
						extension_found = true;
						break;
					}

				if (!extension_found) return false;
			}

			return true;
		}
	};
}

#endif // VIRTUALVISTA_VULKANRENDERERHELPER_H