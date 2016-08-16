
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

/*#define VV_CHECK_ERROR(error, message) { \
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

#ifdef VK_MAKE_VERSION
template <typename T>
class VDeleter {
public:
	VDeleter() : VDeleter([](T _) {}) {}

	VDeleter(std::function<void(T, VkAllocationCallbacks*)> deletef) {
		this->deleter = [=](T obj) { deletef(obj, nullptr); };
	}

	VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef) {
		this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
	}

	VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef) {
		this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
	}

	~VDeleter() {
		cleanup();
	}

	T* operator &() {
		cleanup();
		return &object;
	}

	operator T() const {
		return object;
	}

private:
	T object{ VK_NULL_HANDLE };
	std::function<void(T)> deleter;

	void cleanup() {
		if (object != VK_NULL_HANDLE) {
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
};
#endif

#endif // VIRTUALVISTA_UTILS_H