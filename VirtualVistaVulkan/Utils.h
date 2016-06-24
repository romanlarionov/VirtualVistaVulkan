
#include <iostream>
#include <vulkan\vulkan.h>

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

#define VV_CHECK_ERROR(error, message) { \
        if (VK_SUCCESS != error) \
        { \
            std::cerr << "Vulkan ERROR: " << message << std::endl; \
            std::exit(-1); \
        } \
    }

#endif // VIRTUALVISTA_UTILS_H