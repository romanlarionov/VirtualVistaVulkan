
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <chrono>
#include <array>

#include "DeferredRenderer.h"
#include "Settings.h"
#include "Scene.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WIN32
    #define NOMINMAX  
    #include <Windows.h>
#endif

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
            void *usr_data)
    {
        std::ostringstream stream;
        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
            stream << "WARNING: ";
        if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
            stream << "PERFORMANCE: ";
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            stream << "ERROR: ";
        if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
            stream << "DEBUG: ";

        stream << "@[" << layer_prefix << "]" << std::endl;
        stream << msg << std::endl;
        std::cout << stream.str() << std::endl;

    #ifdef _WIN32
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            MessageBox(NULL, stream.str().c_str(), "VirtualVista Vulkan Error", 0);
    #endif

        return false;
    }

    void DeferredRenderer::createDebugReportCallbackEXT(VkInstance instance, PFN_vkDebugReportCallbackEXT vulkan_debug_callback, const VkAllocationCallbacks* allocator)
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

    void DeferredRenderer::destroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks* allocator)
    {
#ifdef _DEBUG
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func != nullptr)
            func(instance, _debug_callback, allocator);
#endif
    }

	void DeferredRenderer::create(GLFWWindow *window)
	{
        m_window = window;

        createVulkanInstance();
        m_window->createSurface(m_instance);

        DeferredRenderer::createDebugReportCallbackEXT(m_instance, vulkanDebugCallback, nullptr);
        createVulkanDevices();

        _swap_chain.create(&_physical_device, m_window);

        m_render_pass.addAttachment
        (
              _swap_chain.format
            , VK_SAMPLE_COUNT_1_BIT
            , VK_ATTACHMENT_LOAD_OP_CLEAR
            , VK_ATTACHMENT_STORE_OP_STORE
            , VK_ATTACHMENT_LOAD_OP_DONT_CARE
            , VK_ATTACHMENT_STORE_OP_DONT_CARE
            , VK_IMAGE_LAYOUT_UNDEFINED
            , VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        );

        m_render_pass.addAttachment
        (
              _swap_chain.depth_image->format
            , VK_SAMPLE_COUNT_1_BIT
            , VK_ATTACHMENT_LOAD_OP_CLEAR
            , VK_ATTACHMENT_STORE_OP_DONT_CARE
            , VK_ATTACHMENT_LOAD_OP_DONT_CARE
            , VK_ATTACHMENT_STORE_OP_DONT_CARE
            , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        );

        m_render_pass.create(&_physical_device, VK_PIPELINE_BIND_POINT_GRAPHICS);

        for (std::size_t i = 0; i < _swap_chain.color_image_views.size(); ++i)
        {
            std::vector<VkImageView> attachments = { _swap_chain.color_image_views[i]->image_view, _swap_chain.depth_image_view->image_view };
            _frame_buffers.push_back(m_render_pass.createFramebuffer(attachments, _swap_chain.extent));
        }

        _image_ready_semaphore = util::createVulkanSemaphore(this->_physical_device.logical_device);
        _rendering_complete_semaphore = util::createVulkanSemaphore(this->_physical_device.logical_device);

        m_scene.create(&_physical_device, &m_render_pass);
	}


	void DeferredRenderer::shutDown()
	{
        // accounts for the issue of a logical device that might be executing commands when a terminating command is issued.
        vkDeviceWaitIdle(_physical_device.logical_device);

        vkDestroySemaphore(_physical_device.logical_device, _image_ready_semaphore, nullptr);
        vkDestroySemaphore(_physical_device.logical_device, _rendering_complete_semaphore, nullptr);

        m_scene.shutDown();

        m_render_pass.shutDown();

        for (std::size_t j = 0; j < _frame_buffers.size(); ++j)
            vkDestroyFramebuffer(_physical_device.logical_device, _frame_buffers[j], nullptr);

        _swap_chain.shutDown(&_physical_device);

        _physical_device.shutDown();

        m_window->shutDown(m_instance);
        DeferredRenderer::destroyDebugReportCallbackEXT(m_instance, nullptr);
        vkDestroyInstance(m_instance, nullptr);
	}


	void DeferredRenderer::run(float delta_time)
	{
        m_scene.updateUniformData(_swap_chain.extent, delta_time);

        // Draw Frame
        /// Acquire an image from the swap chain
        uint32_t image_index = 0;
        _swap_chain.acquireNextImage(&_physical_device, _image_ready_semaphore, image_index);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        /// tell the queue to wait until a command buffer successfully attaches a swap chain image as a color attachment (wait until its ready to begin rendering).
        std::array<VkPipelineStageFlags, 1> wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &_image_ready_semaphore;
        submit_info.pWaitDstStageMask = wait_stages.data();

        /// Set the command buffer that will be used to rendering to be the one we waited for.
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &_command_buffers[image_index];

        /// Detail the semaphore that marks when rendering is complete.
        std::array<VkSemaphore, 1> signal_semaphores = { _rendering_complete_semaphore };
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores.data();

        VV_CHECK_SUCCESS(vkQueueSubmit(_physical_device.graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
        _swap_chain.present(_physical_device.graphics_queue, image_index, _rendering_complete_semaphore);
	}


    void DeferredRenderer::recordCommandBuffers()
    {
        _command_buffers.resize(_frame_buffers.size());

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = _physical_device.command_pools["graphics"];

        // primary can be sent to pool for execution, but cant be called from other buffers. secondary cant be sent to pool, but can be called from other buffers.
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
        command_buffer_allocate_info.commandBufferCount = static_cast<uint32_t>(_command_buffers.size());

        VV_CHECK_SUCCESS(vkAllocateCommandBuffers(_physical_device.logical_device, &command_buffer_allocate_info, _command_buffers.data()));

        std::vector<VkClearValue> clear_values;
        VkClearValue color_value, depth_value;
        color_value.color = { 0.3f, 0.5f, 0.5f, 1.0f };
        depth_value.depthStencil = {1.0f, 0};
        clear_values.push_back(color_value);
        clear_values.push_back(depth_value);

        m_scene.allocateSceneDescriptorSets();

        for (std::size_t i = 0; i < _command_buffers.size(); ++i)
        {
            VkCommandBufferBeginInfo command_buffer_begin_info = {};
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // <- tells how long this buffer will be executed
            command_buffer_begin_info.pInheritanceInfo = nullptr; // for if this is a secondary buffer
            VV_CHECK_SUCCESS(vkBeginCommandBuffer(_command_buffers[i], &command_buffer_begin_info));

            m_render_pass.beginRenderPass(_command_buffers[i], VK_SUBPASS_CONTENTS_INLINE, _frame_buffers[i], _swap_chain.extent, clear_values);

            m_scene.render(_command_buffers[i]);

            m_render_pass.endRenderPass(_command_buffers[i]);
            VV_CHECK_SUCCESS(vkEndCommandBuffer(_command_buffers[i]));
        }
    }


    Scene* DeferredRenderer::getScene() const
    {
        return (Scene*)&m_scene;
    }


	bool DeferredRenderer::shouldStop()
	{
        return m_window->shouldClose();
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
	void DeferredRenderer::createVulkanInstance()
	{
        VV_ASSERT(checkValidationLayerSupport(), "Validation layers requested are not available on this system.");

        // Instance Creation
        std::string application_name = Settings::inst()->getApplicationName();
        std::string engine_name = Settings::inst()->getEngineName();

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = application_name.c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = engine_name.c_str();
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        // Ensure required extensions are found.
        VV_ASSERT(checkInstanceExtensionSupport(), "Extensions requested, but are not available on this system.");

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;

        auto required_extensions = getRequiredExtensions();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
        instance_create_info.ppEnabledExtensionNames = required_extensions.data();

#ifdef _DEBUG
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(_used_validation_layers.size());
        instance_create_info.ppEnabledLayerNames = const_cast<const char* const*>(_used_validation_layers.data());
#else
        instance_create_info.enabledLayerCount = 0;
#endif

        VV_CHECK_SUCCESS(vkCreateInstance(&instance_create_info, nullptr, &m_instance));
	}


	std::vector<const char*> DeferredRenderer::getRequiredExtensions()
	{
        std::vector<const char*> extensions;

        // GLFW specific
        for (uint32_t i = 0; i < m_window->glfw_extension_count; i++)
            extensions.push_back(m_window->glfw_extensions[i]);

        // System wide required (hardcoded)
        for (auto extension : _used_instance_extensions)
            extensions.push_back(extension);

        return extensions;
	}


	bool DeferredRenderer::checkInstanceExtensionSupport()
	{
        std::vector<const char*> required_extensions = getRequiredExtensions();

        uint32_t extension_count = 0;
        VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        VV_CHECK_SUCCESS(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data()));

        // Compare found extensions with requested ones.
        for (const auto& extension : required_extensions)
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


	bool DeferredRenderer::checkValidationLayerSupport()
	{
        uint32_t layer_count = 0;
        VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
        std::vector<VkLayerProperties> available_layers(layer_count);
        VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()));

        // Compare found layers with requested ones.
        for (const char* layer : _used_validation_layers)
        {
            bool layer_found = false;
            for (const auto& found_layer : available_layers)
                if (strcmp(layer, found_layer.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }

            if (!layer_found) return false;
        }

        return true;
	}


    bool DeferredRenderer::isVulkanDeviceSuitable(VulkanDevice &device)
    {
        return device.hasGraphicsQueue() && device.querySwapChainSupport(m_window->surface).is_supported;
    }


    void DeferredRenderer::createVulkanDevices()
    {
        uint32_t _physical_device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &_physical_device_count, nullptr);

        VV_ASSERT(_physical_device_count != 0, "Vulkan Error: no gpu with Vulkan support found");

        std::vector<VkPhysicalDevice> physical_devices(_physical_device_count);
        vkEnumeratePhysicalDevices(m_instance, &_physical_device_count, physical_devices.data());

        bool found = false;

        // Find any physical devices that might be suitable for on screen rendering.
        for (const auto& device : physical_devices)
        {
            _physical_device.create(device);
            if (isVulkanDeviceSuitable(_physical_device))
            {
                if (_physical_device.hasTransferQueue())
                    _physical_device.createLogicalDevice(true, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
                else
                    _physical_device.createLogicalDevice(true, VK_QUEUE_GRAPHICS_BIT);

                m_window->surface_settings[&_physical_device] = _physical_device.querySwapChainSupport(m_window->surface);
                found = true;
                break;
            }
            else
            {
                _physical_device.shutDown();
            }
        }

        VV_ASSERT(found, "Vulkan Error: no gpu with Vulkan support found");
    }
}