
#ifndef VIRTUALVISTA_RENDERER_H
#define VIRTUALVISTA_RENDERER_H

#include "Scene.h"
#include "GLFWWindow.h"
#include "Utils.h"

namespace vv
{
    class Renderer
    {
    public:
        Renderer() = default;
        virtual ~Renderer() = default;

        /*
         * Initialize all necessary Vulkan internals.
         */
        virtual void create(GLFWWindow *window) = 0;

        /*
         * Destroy all Vulkan internals in the correct order.
         * 
         * note: this needs to be called at application termination to ensure that all corresponding Vulkan pieces
         *       are destroyed in the proper order. This is needed because of C++'s lack of destructor guarantees.
         */
        virtual void shutDown() = 0;

        /*
         * Submits all command buffers to the various vulkan queues for execution.
         */
        virtual void run(float delta_time) = 0;

        /*
         * Signals the renderer to start recording vulkan command buffers once the scene has been properly populated.
         */
        virtual void recordCommandBuffers() = 0;

        /*
         * Returns a scene object used to store entities with physical presence.
         */
        virtual Scene* getScene() const = 0;

        /*
         * Returns whether the renderer should stop execution.
         */
        virtual bool shouldStop() = 0;

    protected:
        VkDebugReportCallbackEXT _debug_callback = VK_NULL_HANDLE;

        void createDebugReportCallbackEXT(VkInstance instance, PFN_vkDebugReportCallbackEXT vulkan_debug_callback, const VkAllocationCallbacks* allocator)
        {
#ifdef _DEBUG
            auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

            VkDebugReportCallbackCreateInfoEXT _debug_callback_create_info = {};
            _debug_callback_create_info.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            _debug_callback_create_info.pfnCallback = vulkan_debug_callback;
            _debug_callback_create_info.flags       = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                                     VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                                     VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                                     VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                                     VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;

            VkResult result = VK_ERROR_EXTENSION_NOT_PRESENT;
            if (func != nullptr)
                result = func(instance, &_debug_callback_create_info, allocator, &_debug_callback);

            VV_CHECK_SUCCESS(result);
#endif
        }

        void destroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks* allocator)
        {
#ifdef _DEBUG
            auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
            if (func != nullptr)
                func(instance, _debug_callback, allocator);
#endif
        }
    };
}

#endif // VIRTUALVISTA_RENDERER_H