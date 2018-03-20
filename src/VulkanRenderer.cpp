
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <chrono>
#include <array>

#include "VulkanRenderer.h"
#include "Settings.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WIN32
    #define NOMINMAX  
    #include <Windows.h>
#endif

namespace vv
{
	VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pCallback);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}


	void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
#ifdef _DEBUG
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
			func(instance, callback, pAllocator);
#endif
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanRenderer::VulkanRenderer()
	{
	}


	VulkanRenderer::~VulkanRenderer()
	{
	}


	void VulkanRenderer::create(GLFWWindow *window)
	{
        _window = window;

        createVulkanInstance();
        _window->createSurface(_instance);

        setupDebugCallback();
        createVulkanDevices();

        _swap_chain = new VulkanSwapChain();
        _swap_chain->create(_physical_device, _window);

        _render_pass = new VulkanRenderPass();
        _render_pass->create(_physical_device, _swap_chain);

        createFrameBuffers();

        _image_ready_semaphore = util::createVulkanSemaphore(this->_physical_device->logical_device);
        _rendering_complete_semaphore = util::createVulkanSemaphore(this->_physical_device->logical_device);

        _scene = new Scene();
        _scene->create(_physical_device, _render_pass);

	}


	void VulkanRenderer::shutDown()
	{
        // accounts for the issue of a logical device that might be executing commands when a terminating command is issued.
        vkDeviceWaitIdle(_physical_device->logical_device);

        vkDestroySemaphore(_physical_device->logical_device, _image_ready_semaphore, nullptr);
        vkDestroySemaphore(_physical_device->logical_device, _rendering_complete_semaphore, nullptr);

        _scene->shutDown();

        _render_pass->shutDown();
        delete _render_pass;

        for (std::size_t j = 0; j < _frame_buffers.size(); ++j)
            vkDestroyFramebuffer(_physical_device->logical_device, _frame_buffers[j], nullptr);

        _swap_chain->shutDown(_physical_device);
        delete _swap_chain;

        _physical_device->shutDown();
        delete _physical_device;

        _window->shutDown(_instance);
        destroyDebugReportCallbackEXT(_instance, _debug_callback, nullptr);
        vkDestroyInstance(_instance, nullptr);
	}


	void VulkanRenderer::run(float delta_time)
	{
        _scene->updateUniformData(_swap_chain->extent, delta_time);

        // Draw Frame
        /// Acquire an image from the swap chain
        uint32_t image_index = 0;
        _swap_chain->acquireNextImage(_physical_device, _image_ready_semaphore, image_index);

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

        VV_CHECK_SUCCESS(vkQueueSubmit(_physical_device->graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
        _swap_chain->queuePresent(_physical_device->graphics_queue, image_index, _rendering_complete_semaphore);
	}


    void VulkanRenderer::recordCommandBuffers()
    {
        _command_buffers.resize(_frame_buffers.size());

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = _physical_device->command_pools["graphics"];

        // primary can be sent to pool for execution, but cant be called from other buffers. secondary cant be sent to pool, but can be called from other buffers.
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
        command_buffer_allocate_info.commandBufferCount = static_cast<uint32_t>(_command_buffers.size());

        VV_CHECK_SUCCESS(vkAllocateCommandBuffers(_physical_device->logical_device, &command_buffer_allocate_info, _command_buffers.data()));

        std::vector<VkClearValue> clear_values;
        VkClearValue color_value, depth_value;
        color_value.color = { 0.3f, 0.5f, 0.5f, 1.0f };
        depth_value.depthStencil = {1.0f, 0};
        clear_values.push_back(color_value);
        clear_values.push_back(depth_value);

        _scene->allocateSceneDescriptorSets();

        for (std::size_t i = 0; i < _command_buffers.size(); ++i)
        {
            VkCommandBufferBeginInfo command_buffer_begin_info = {};
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // <- tells how long this buffer will be executed
            command_buffer_begin_info.pInheritanceInfo = nullptr; // for if this is a secondary buffer
            VV_CHECK_SUCCESS(vkBeginCommandBuffer(_command_buffers[i], &command_buffer_begin_info));

            _render_pass->beginRenderPass(_command_buffers[i], VK_SUBPASS_CONTENTS_INLINE, _frame_buffers[i], _swap_chain->extent, clear_values);

            _scene->render(_command_buffers[i]);

            _render_pass->endRenderPass(_command_buffers[i]);
            VV_CHECK_SUCCESS(vkEndCommandBuffer(_command_buffers[i]));
        }
    }


    Scene* VulkanRenderer::getScene() const
    {
        return _scene;
    }


	bool VulkanRenderer::shouldStop()
	{
        return _window->shouldClose();
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
	void VulkanRenderer::createVulkanInstance()
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

        VkInstanceCreateInfo _instancecreate_info = {};
        _instancecreate_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        _instancecreate_info.pApplicationInfo = &app_info;

        auto required_extensions = getRequiredExtensions();
        _instancecreate_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
        _instancecreate_info.ppEnabledExtensionNames = required_extensions.data();

#ifdef _DEBUG
        _instancecreate_info.enabledLayerCount = static_cast<uint32_t>(_used_validation_layers.size());
        _instancecreate_info.ppEnabledLayerNames = const_cast<const char* const*>(_used_validation_layers.data());
#else
        _instancecreate_info.enabledLayerCount = 0;
#endif

        VV_CHECK_SUCCESS(vkCreateInstance(&_instancecreate_info, nullptr, &_instance));
	}


	std::vector<const char*> VulkanRenderer::getRequiredExtensions()
	{
        std::vector<const char*> extensions;

        // GLFW specific
        for (uint32_t i = 0; i < _window->glfw_extension_count; i++)
            extensions.push_back(_window->glfw_extensions[i]);

        // System wide required (hardcoded)
        for (auto extension : _used_instance_extensions)
            extensions.push_back(extension);

        return extensions;
	}


	bool VulkanRenderer::checkInstanceExtensionSupport()
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


	bool VulkanRenderer::checkValidationLayerSupport()
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


    void VulkanRenderer::setupDebugCallback()
    {
#ifdef _DEBUG
        VkDebugReportCallbackCreateInfoEXT _debug_callbackcreate_info = {};
        _debug_callbackcreate_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        _debug_callbackcreate_info.pfnCallback = vulkanDebugCallback;
        _debug_callbackcreate_info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                            VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                            VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                            VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                            VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;

        VV_CHECK_SUCCESS(createDebugReportCallbackEXT(_instance, &_debug_callbackcreate_info, nullptr, &_debug_callback));
#endif
	}


    void VulkanRenderer::createVulkanDevices()
    {
        uint32_t _physical_devicecount = 0;
        vkEnumeratePhysicalDevices(_instance, &_physical_devicecount, nullptr);

        VV_ASSERT(_physical_devicecount != 0, "Vulkan Error: no gpu with Vulkan support found");

        std::vector<VkPhysicalDevice> physical_devices(_physical_devicecount);
        vkEnumeratePhysicalDevices(_instance, &_physical_devicecount, physical_devices.data());

        // Find any physical devices that might be suitable for on screen rendering.
        for (const auto& device : physical_devices)
        {
            _physical_device = new VulkanDevice;
            _physical_device->create(device);
            VulkanSurfaceDetailsHandle surface_details_handle = {};
            if (_physical_device->isSuitable(_window->surface, surface_details_handle))
            {
                _physical_device->createLogicalDevice(true, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
                _window->surface_settings[_physical_device] = surface_details_handle;
                break;
            }
            else
            {
                _physical_device->shutDown();
                delete _physical_device;
                _physical_device = nullptr;
            }
        }

        VV_ASSERT(_physical_device != nullptr, "Vulkan Error: no gpu with Vulkan support found");
    }


    void VulkanRenderer::createFrameBuffers()
    {
        _frame_buffers.resize(_swap_chain->color_image_views.size());

        for (std::size_t i = 0; i < _swap_chain->color_image_views.size(); ++i)
        {
            std::vector<VkImageView> attachments = { _swap_chain->color_image_views[i]->image_view, _swap_chain->depth_image_view->image_view };

            VkFramebufferCreateInfo frame_buffer_create_info = {};
            frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frame_buffer_create_info.flags = 0;
            frame_buffer_create_info.renderPass = _render_pass->render_pass;
            frame_buffer_create_info.attachmentCount = (uint32_t)attachments.size();
            frame_buffer_create_info.pAttachments = attachments.data();
            frame_buffer_create_info.width = _swap_chain->extent.width;
            frame_buffer_create_info.height = _swap_chain->extent.height;
            frame_buffer_create_info.layers = 1;

            VV_CHECK_SUCCESS(vkCreateFramebuffer(_physical_device->logical_device, &frame_buffer_create_info, nullptr, &_frame_buffers[i]));
        }
    }
}