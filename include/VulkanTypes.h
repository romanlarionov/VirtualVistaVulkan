
#ifndef VIRTUALVISTA_VULKANTYPES_H
#define VIRTUALVISTA_VULKANTYPES_H

#include <vulkan\vulkan.h>
#include <vector>

struct VulkanSurfaceDetailsHandle
{
	VkSurfaceCapabilitiesKHR surface_capabilities;
	std::vector<VkSurfaceFormatKHR> available_surface_formats;
	std::vector<VkPresentModeKHR> available_surface_present_modes;
};

#endif // VIRTUALVISTA_VULKANTYPES_H
