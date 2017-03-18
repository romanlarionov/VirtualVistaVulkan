
#ifndef VIRTUALVISTA_GLFWWINDOW_H
#define VIRTUALVISTA_GLFWWINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <vector>

#include "InputManager.h"

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
        friend class InputManager;

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
         *
         * todo: add resizing event handler. requires manually updating framebuffer
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

	private:

        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

        static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    };
}

#endif // VIRTUALVISTA_GLFWWINDOW_H