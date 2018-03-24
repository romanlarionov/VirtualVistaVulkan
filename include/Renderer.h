
#ifndef VIRTUALVISTA_RENDERER_H
#define VIRTUALVISTA_RENDERER_H

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


    enum RendererType
    {
          VULKAN_RENDERER_TYPE_FORWARD = 0
        , VULKAN_RENDERER_TYPE_DEFERRED = 1
    };

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

        void createDebugReportCallbackEXT(VkInstance instance, PFN_vkDebugReportCallbackEXT vulkan_debug_callback, const VkAllocationCallbacks* allocator);
        void destroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks* allocator);

    };
}

#endif // VIRTUALVISTA_RENDERER_H