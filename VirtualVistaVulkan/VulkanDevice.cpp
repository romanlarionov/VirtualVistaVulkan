
#include "VulkanDevice.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanDevice::VulkanDevice()
	{
	}


	VulkanDevice::~VulkanDevice()
	{
	}


	void VulkanDevice::create(VkPhysicalDevice device)
	{
		physical_device = device;
		VV_ASSERT(physical_device != VK_NULL_HANDLE, "Vulkan Physical Device NULL");

		// Query and format all data related to this GPU.
		vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
		vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);
		vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

		uint32_t queue_family_properties_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);
		VV_ASSERT(queue_family_properties_count > 0, "vkGetPhysicalDeviceQueueFamilyProperties returned 0");
		queue_family_properties.resize(queue_family_properties_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, queue_family_properties.data());

		queryQueueFamilies();
	}


	void VulkanDevice::shutDown()
	{
		if (logical_device != VK_NULL_HANDLE)
		{
			// Command Pool/Buffers

			for (auto &pool : command_pools)
				vkDestroyCommandPool(logical_device, pool.second, nullptr);

			vkDestroyDevice(logical_device, nullptr);
		}
	}

	
	bool VulkanDevice::isSuitable(VkSurfaceKHR surface, VulkanSurfaceDetailsHandle &surface_details)
	{
		if (surface == VK_NULL_HANDLE) return false;
		bool result = true;
		
		if (Settings::inst()->isGraphicsRequired() && graphics_family_index < 0)
			result = false;
		if (Settings::inst()->isComputeRequired() && compute_family_index < 0)
			result = false;
		if (Settings::inst()->isOnScreenRenderingRequired() && !querySwapChainSupport(surface, surface_details))
			result = false;
	
		return result;
	}


	void VulkanDevice::createLogicalDevice(bool swap_chain_support, VkQueueFlags queue_types)
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

		if (queue_types & VK_QUEUE_TRANSFER_BIT)
		{
			VkDeviceQueueCreateInfo queue_create_info = {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = transfer_family_index;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &default_queue_priority;
			device_queue_create_infos.push_back(queue_create_info);
		}

		// device extensions
		std::vector<const char*> device_extensions;
		if (swap_chain_support && checkDeviceExtensionSupport(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.flags = 0;
		device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
		device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size());
		device_create_info.pEnabledFeatures = &physical_device_features;

		if (!device_extensions.empty())
		{
			device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
			device_create_info.ppEnabledExtensionNames = device_extensions.data();
		}
		else
			device_create_info.enabledExtensionCount = 0;

#ifdef _DEBUG
		// todo: currently using the same layers for the device that I do for the instance level. might
		// need to diverge here (but it's my understanding that device layers are becoming deprecated).
		device_create_info.enabledLayerCount = static_cast<uint32_t>(used_validation_layers_.size());
		device_create_info.ppEnabledLayerNames = used_validation_layers_.data();
#else
		device_create_info.enabledLayerCount = 0;
#endif

		VV_CHECK_SUCCESS(vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device));

		// Set up queue handles.
		if (graphics_family_index >= 0)
			vkGetDeviceQueue(logical_device, graphics_family_index, 0, &graphics_queue);

		if (compute_family_index >= 0)
			vkGetDeviceQueue(logical_device, compute_family_index, 0, &compute_queue);

		if (transfer_family_index >= 0)
			vkGetDeviceQueue(logical_device, transfer_family_index, 0, &transfer_queue);

		// Set up default command pools.
		createCommandPool("graphics", graphics_family_index, 0);
		createCommandPool("transfer", transfer_family_index, 0);
	}

	
	void VulkanDevice::createCommandPool(std::string name, uint32_t queue_index, VkCommandPoolCreateFlags create_flags)
	{
		VkCommandPoolCreateInfo command_pool_create_info = {};
		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.flags = create_flags; // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT <- tells vulkan that command buffers will change frequently
		command_pool_create_info.queueFamilyIndex = queue_index;

		VkCommandPool command_pool = VK_NULL_HANDLE;
		VV_CHECK_SUCCESS(vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, &command_pool));

		command_pools[name] = command_pool;
	}

	
	uint32_t VulkanDevice::findMemoryTypeIndex(uint32_t filter_type, VkMemoryPropertyFlags memory_property_flags)
	{
		// todo: check for validity and robustness.
		auto memory_properties = physical_device_memory_properties;
		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
		{
			// Check all memory heaps to find memory type that fulfills out needs and has the correct properties.
			if ((filter_type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags)
				return i;
		}

		VV_ASSERT(false, "Couldn't find appropriate memory type");
		return 0;
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////// Private
	bool VulkanDevice::querySwapChainSupport(VkSurfaceKHR surface, VulkanSurfaceDetailsHandle &surface_details_handle)
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

	
	void VulkanDevice::queryQueueFamilies()
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
				if (family.queueFlags & VK_QUEUE_TRANSFER_BIT && !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) 
					transfer_family_index = i;
			}
			i++;
		}
	}

	
	bool VulkanDevice::checkDeviceExtensionSupport(const char* extension)
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
}