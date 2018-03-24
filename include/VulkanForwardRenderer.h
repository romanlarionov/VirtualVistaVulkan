
#ifndef VIRTUALVISTA_VULKANFORWARDRENDERER_H
#define VIRTUALVISTA_VULKANFORWARDRENDERER_H

#include <vector>
#include <array>

#include "Renderer.h"
#include "GLFWWindow.h"
#include "VulkanSwapChain.h"
#include "VulkanImageView.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "Scene.h"
#include "Material.h"
#include "ModelManager.h"
#include "Mesh.h"
#include "Utils.h"

namespace vv
{
    class VulkanForwardRenderer : public Renderer
    {
    public:
        VulkanForwardRenderer() = default;
        virtual ~VulkanForwardRenderer() = default;

        /*
         * Initialize all necessary Vulkan internals.
         */
        virtual void create(GLFWWindow *window);

        /*
         * Destroy all Vulkan internals in the correct order.
         * 
         * note: this needs to be called at application termination to ensure that all corresponding Vulkan pieces
         *       are destroyed in the proper order. This is needed because of C++'s lack of destructor guarantees.
         */
        virtual void shutDown();

        /*
         * Submits all command buffers to the various vulkan queues for execution.
         */
        virtual void run(float delta_time);

        /*
         * Signals the renderer to start recording vulkan command buffers once the scene has been properly populated.
         */
        virtual void recordCommandBuffers();

        /*
         * Returns a scene object used to store entities with physical presence.
         */
        virtual Scene* getScene() const;

        /*
         * Returns whether the renderer should stop execution.
         */
        virtual bool shouldStop();

    private:
        GLFWWindow *_window						    = nullptr;
        VkInstance _instance					    = VK_NULL_HANDLE;
        VulkanDevice _physical_device;
    
        VulkanSwapChain _swap_chain;
        std::vector<VkFramebuffer> _frame_buffers;

        VkSemaphore _image_ready_semaphore          = VK_NULL_HANDLE;
        VkSemaphore _rendering_complete_semaphore   = VK_NULL_HANDLE;

        VulkanRenderPass _render_pass;
        std::vector<VkCommandBuffer> _command_buffers;

        Scene _scene;

        std::vector<const char*> _used_validation_layers = { "VK_LAYER_LUNARG_standard_validation" };
        const std::vector<const char*> _used_instance_extensions = { VK_EXT_DEBUG_REPORT_EXTENSION_NAME };

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
    };
}

#endif // VIRTUALVISTA_VULKANFORWARDRENDERER_H