
#include <iostream>
#include <string>

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

/*#define VV_CHECK_SUCCESS(success, message) { \
        if (success != VK_SUCCESS) \
        { \
            std::cerr << "Vulkan ERROR: " << message << std::endl; \
            std::exit(-1); \
        } \
    }

#define VV_CHECK_ERROR(error, message) { \
        if (error == NULL || error == true) \
        { \
            std::cerr << "Vulkan ERROR: " << message << std::endl; \
            std::exit(-1); \
        } \
    }
*/

enum RENDERER_TYPE {
	VULKAN,
	OPENGL,
	DIRECT3D
};

enum WINDOW_TYPE {
	GLFW,
	SDL,
	WINDOWS,
	X11
};

#endif // VIRTUALVISTA_UTILS_H