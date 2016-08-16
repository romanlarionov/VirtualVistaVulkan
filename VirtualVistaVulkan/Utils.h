
#include <iostream>
#include <string>
#include <functional>
#include <vulkan\vulkan.h>

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

#define VV_CHECK_SUCCESS(success, message) { \
        if (success != VK_SUCCESS) \
        { \
			throw std::runtime_error(message); \
        } \
    }

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