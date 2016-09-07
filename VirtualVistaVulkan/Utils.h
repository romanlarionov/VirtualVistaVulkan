
#include <iostream>
#include <string>
#include <functional>

// todo: this might break on a system with no vulkan support. find an ifdef guard that will prevent this.
#include <vulkan\vulkan.h>

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

#define VV_SAFE_DELETE(p) { \
		if (!p) {} \
		else { delete p; p = NULL; } \
	}

#ifdef _DEBUG

#define VV_CHECK_SUCCESS(success) { \
        if (success == VK_SUCCESS) { } \
		else throw std::runtime_error(__FUNCTION__  + std::to_string(__LINE__) + __FILE__); \
    }

#define VV_ASSERT(condition, message) \
		if (condition) { } \
		else \
		{ \
			throw std::runtime_error(#message + std::to_string(__LINE__) + __FILE__ ); \
		}


#else

#define VV_CHECK_SUCCESS(success) {}
#define VV_ASSERT(condition, message) {}

#endif

namespace vv
{
	enum RENDERER_TYPE
	{
		VULKAN,
		OPENGL,
		DIRECT3D
	};

	enum WINDOW_TYPE
	{
		GLFW,
		SDL,
		WINDOWS,
		X11
	};
}

#endif // VIRTUALVISTA_UTILS_H