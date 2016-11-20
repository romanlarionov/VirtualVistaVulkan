
#include <iostream>
#include <string>
#include <functional>
#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

#define VV_SAFE_DELETE(p) { \
		if (!p) {} \
		else { delete p; p = NULL; } \
	}

#ifdef _DEBUG

#define VV_CHECK_SUCCESS(success) { \
        if (success == VK_SUCCESS) { } \
		else throw std::runtime_error(__FUNCTION__ + std::to_string(__LINE__) + " " + __FILE__); \
    }

#define VV_ASSERT(condition, message) \
		if (condition) { } \
		else \
		{ \
			throw std::runtime_error(#message + std::to_string(__LINE__) + " " + __FILE__ ); \
		}

#else

#define VV_CHECK_SUCCESS(success) {}
#define VV_ASSERT(condition, message) {}

#endif

namespace vv
{
	// todo: rework this horrible atrocity
	struct Vertex
	{
	public:
		glm::vec2 position;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDesciption()
		{
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions;

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0; // layout placement
			attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // type
			attribute_descriptions[0].offset = offsetof(Vertex, position); // placement in vertex 

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(Vertex, color);

			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[2].offset = offsetof(Vertex, texCoord);

			return attribute_descriptions;
		}
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec3 normal;
	};

	static VkCommandBuffer beginSingleUseCommand(VkDevice device, VkCommandPool command_pool)
	{
		VkCommandBufferAllocateInfo allocate_info = {};
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocate_info.commandPool = command_pool;
    	allocate_info.commandBufferCount = 1;

    	VkCommandBuffer command_buffer;
    	vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);

    	VkCommandBufferBeginInfo begin_info = {};
    	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    	vkBeginCommandBuffer(command_buffer, &begin_info);

    	return command_buffer;
	}

	static void endSingleUseCommand(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer, VkQueue queue)
	{
	    vkEndCommandBuffer(command_buffer);

	    VkSubmitInfo submit_info = {};
	    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	    submit_info.commandBufferCount = 1;
	    submit_info.pCommandBuffers = &command_buffer;

	    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
	    vkQueueWaitIdle(queue);

	    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
	}
}

#endif // VIRTUALVISTA_UTILS_H