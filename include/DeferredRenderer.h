
#ifndef VIRTUALVISTA_DEFERREDRENDERER_H
#define VIRTUALVISTA_DEFERREDRENDERER_H

#include <vector>
#include <array>

#include "Scene.h"
#include "GLFWWindow.h"
#include "Utils.h"

namespace vv
{
    VKAPI_ATTR VkBool32 VKAPI_CALL
        vulkanDebugCallback(VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT obj_type, // object that caused the error
            uint64_t src_obj,
            size_t location,
            int32_t msg_code,
            const char *layer_prefix,
            const char *msg,
            void *usr_data);

    class DeferredRenderer
    {
    public:
        DeferredRenderer() = default;
        ~DeferredRenderer() = default;

        /*
         * Initialize all necessary Vulkan internals.
         */
        void create(GLFWWindow *window);

        /*
         * Destroy all Vulkan internals in the correct order.
         * 
         * note: this needs to be called at application termination to ensure that all corresponding Vulkan pieces
         *       are destroyed in the proper order. This is needed because of C++'s lack of destructor guarantees.
         */
        void shutDown();

        /*
         * Submits all command buffers to the various vulkan queues for execution.
         */
        void run(float delta_time);

        /*
         * Signals the renderer to start recording vulkan command buffers once the scene has been properly populated.
         */
        void recordCommandBuffers();

        /*
         * Returns a scene object used to store entities with physical presence.
         */
        Scene* getScene() const;

        /*
         * Returns whether the renderer should stop execution.
         */
        bool shouldStop();

    protected:
        VkDebugReportCallbackEXT _debug_callback = VK_NULL_HANDLE;

        void createDebugReportCallbackEXT(VkInstance instance, PFN_vkDebugReportCallbackEXT vulkan_debug_callback, const VkAllocationCallbacks* allocator);
        void destroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks* allocator);

        GLFWWindow *m_window						    = nullptr;
        VkInstance m_instance					    = VK_NULL_HANDLE;
        VulkanDevice m_physical_device;
    
        VulkanSwapChain m_swap_chain;
        std::vector<VkFramebuffer> m_frame_buffers;

        VkSemaphore m_image_ready_semaphore          = VK_NULL_HANDLE;
        VkSemaphore m_rendering_complete_semaphore   = VK_NULL_HANDLE;

        VulkanRenderPass m_render_pass;
        std::vector<VkCommandBuffer> m_command_buffers;

        Scene m_scene;

        std::vector<const char*> m_used_validation_layers = { "VK_LAYER_LUNARG_standard_validation" };
        const std::vector<const char*> m_used_instance_extensions = { VK_EXT_DEBUG_REPORT_EXTENSION_NAME };

        /*
         * Creates the main Vulkan instance upon which the renderer rests.
         */
        void createVulkanInstance();

        /*
         * Creates device handles for any GPUs found with Vulkan support.
         */
        void createVulkanDevices();

        /*
         * Returns back a formatted list of all extensions used by the system.
         */
        std::vector<const char*> getRequiredExtensions();

        /*
         * Checks to see if the required 3rd party Vulkan extensions are available on the current system
         */
        bool checkInstanceExtensionSupport();

        /* 
         * Checks to see if the validation layers that were requested are available on the current system
         * FOR DEBUGGING PURPOSES ONLY 
         */
        bool checkValidationLayerSupport();

        /*
         * Checks to see if all of the requirements for this type of renderer are met with any detected hardware devices.
         */
        bool isVulkanDeviceSuitable(VulkanDevice &device);

        /*
         * Creates the 
         */
        void createFullscreenQuad();

    };
}

#endif // VIRTUALVISTA_DEFERREDRENDERER_H