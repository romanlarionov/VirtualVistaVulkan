
#include "Renderer.h"

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


    void Renderer::createDebugReportCallbackEXT(VkInstance instance, PFN_vkDebugReportCallbackEXT vulkan_debug_callback, const VkAllocationCallbacks* allocator)
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


    void Renderer::destroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks* allocator)
    {
#ifdef _DEBUG
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func != nullptr)
            func(instance, _debug_callback, allocator);
#endif
    }
}