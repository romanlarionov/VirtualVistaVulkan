
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

#define VV_SAFE_DELETE(p) { \
		if (!p) {} \
		else { delete p; p = NULL; } \
	}

namespace vv
{
#ifdef _DEBUG

	inline void VV_CHECK_SUCCESS(VkResult success)
	{
		if (success != VK_SUCCESS)
			throw std::runtime_error(std::to_string(__LINE__) + " " + __FILE__);
	}

	inline void VV_ASSERT(bool condition, std::string message)
	{
		if (!condition)
			throw std::runtime_error(message + std::to_string(__LINE__) + " " + __FILE__);
	}

	inline void VV_ASSERT(VkResult condition, std::string message)
	{
		if (condition != VK_SUCCESS)
			throw std::runtime_error(message + std::to_string(__LINE__) + " " + __FILE__);
	}

#else

	inline void VV_CHECK_SUCCESS(VkResult success) {}
	inline void VV_ASSERT(bool condition, std::string message) {}
	inline void VV_ASSERT(VkResult condition, std::string message) {}

#endif

	struct Vertex
	{
	public:
		glm::vec3 position;
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
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // type
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

		bool operator==(const Vertex& other) const
		{
			return position == other.position && color == other.color && texCoord == other.texCoord;
		}
	};

	struct UniformBufferObject
    {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

    struct MaterialConstants
    {
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        int shininess;
    };

	namespace util
	{
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

		static VkSemaphore createVulkanSemaphore(VkDevice device)
		{
			VkSemaphore semaphore;
			VkSemaphoreCreateInfo semaphore_create_info = {};
			semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VV_CHECK_SUCCESS(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphore));
			return semaphore;
		}

		static void destroyVulkanSemaphore(VkDevice device, VkSemaphore semaphore)
		{
			vkDestroySemaphore(device, semaphore, nullptr);
		}

        static VkDescriptorSetLayout createVulkanDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings)
        {
            VkDescriptorSetLayout layout = {};

            VkDescriptorSetLayoutCreateInfo layout_create_info = {};
		    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		    layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
		    layout_create_info.pBindings = bindings.data();

		    VV_CHECK_SUCCESS(vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &layout));
            return layout;
        }

        static void destroyVulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout)
        {
		    vkDestroyDescriptorSetLayout(device, layout, nullptr);
        }
	}
}

namespace std {
	template<> struct hash<vv::Vertex> {
		size_t operator()(vv::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

#endif // VIRTUALVISTA_UTILS_H