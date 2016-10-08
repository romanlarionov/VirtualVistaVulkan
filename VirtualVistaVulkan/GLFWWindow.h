
#ifndef VIRTUALVISTA_GLFWWINDOW_H
#define VIRTUALVISTA_GLFWWINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <vector>

namespace vv 
{
	struct VulkanDevice;

	struct VulkanSurfaceDetailsHandle
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		std::vector<VkSurfaceFormatKHR> available_surface_formats;
		std::vector<VkPresentModeKHR> available_surface_present_modes;
	};

    class GLFWWindow
    {
    public:
		GLFWwindow *window; // GLFW typedef
		VkSurfaceKHR surface;
		std::unordered_map<VulkanDevice*, VulkanSurfaceDetailsHandle> surface_settings;
		uint32_t glfw_extension_count;
		const char** glfw_extensions;

        GLFWWindow();
        ~GLFWWindow();

		/*
		 * Initializes GLFW and creates the window wrapper.
		 */
		void create();

		/*
		 * Creates a Vulkan surface for generically communicating between Vulkan and the system window API.
		 */
		void createSurface(VkInstance instance);

		/*
		 * Deletes all necessary Vulkan and GLFW containers.
		 */
		void shutDown(VkInstance instance);

		/*
		 * Execute main GLFW polling code.
		 */
		void run();

		/*
		 * Returns whether GLFW received a termination signal.
		 */
		bool shouldClose();

		/*
		 *
		 */
		void addSurfaceDetails(VulkanDevice *device, VulkanSurfaceDetailsHandle details);

		/*
		 *
		 */
		void removeSurfaceDetails(VulkanDevice *device);

	private:

    };
}

#endif // VIRTUALVISTA_GLFWWINDOW_H