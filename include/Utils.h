
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

//#include "Settings.h"

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

#define VV_SAFE_DELETE(p) { \
		if (!p) {} \
		else { delete p; p = NULL; } \
	}

#ifdef _WIN32
    #define NOMINMAX  
    #include <Windows.h>
#endif

namespace vv
{
#ifdef _DEBUG

	inline void VV_CHECK_SUCCESS(VkResult success)
	{
        if (success != VK_SUCCESS)
        {
            std::string err = "Vulkan API error code: " + std::to_string(success);
#ifdef _WIN32
			MessageBox(NULL, err.c_str(), "Vulkan API Error", 0);
#endif
            throw std::runtime_error(err);
        }
	}

	inline void VV_ASSERT(bool condition, std::string message)
	{
        if (!condition)
        {
#ifdef _WIN32
			MessageBox(NULL, message.c_str(), "Vulkan API Error", 0);
#endif
            throw std::runtime_error(message);
        }
	}

	inline void VV_ASSERT(VkResult condition, std::string message)
	{
        if (condition != VK_SUCCESS)
        {
#ifdef _WIN32
			MessageBox(NULL, message.c_str(), "Vulkan API Error", 0);
#endif
            throw std::runtime_error(message);
        }
	}

	inline void VV_ALERT(std::string message)
	{
#ifdef _WIN32
        MessageBox(NULL, message.c_str(), "Application Warning", 0);
#endif
	}

#else

	inline void VV_CHECK_SUCCESS(VkResult success) {}
	inline void VV_ASSERT(bool condition, std::string message) {}
	inline void VV_ASSERT(VkResult condition, std::string message) {}
    inline void VV_ALERT(std::string message) {}

#endif

	struct Vertex
	{
	public:
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDesciption()
		{
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		static VkVertexInputAttributeDescription* getAttributeDescriptions()
		{
			VkVertexInputAttributeDescription *attribute_descriptions = new VkVertexInputAttributeDescription[3];

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0; // layout placement
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // type
			attribute_descriptions[0].offset = offsetof(Vertex, position); // placement in vertex 

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(Vertex, normal);

			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[2].offset = offsetof(Vertex, texCoord);

			return attribute_descriptions;
		}

		bool operator==(const Vertex& other) const
		{
			return position == other.position && normal == other.normal && texCoord == other.texCoord;
		}
	};

    struct MaterialProperties
    {
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        int shininess;
    };

    struct DescriptorInfo
    {
        unsigned binding;
        std::string name;
        VkShaderStageFlagBits shader_stage;
        VkDescriptorType type;
    };

    struct VulkanSurfaceDetailsHandle
    {
        bool is_supported = false;
    	VkSurfaceCapabilitiesKHR surface_capabilities;
    	std::vector<VkSurfaceFormatKHR> available_surface_formats;
    	std::vector<VkPresentModeKHR> available_surface_present_modes;
    };

	namespace util
	{
	    static bool checkInstanceExtensionSupport(std::vector<const char*> &required_extensions)
	    {
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

	    static bool checkValidationLayerSupport(std::vector<const char*> &used_validation_layers)
	    {
#ifdef _DEBUG
            uint32_t layer_count = 0;
            VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
            std::vector<VkLayerProperties> available_layers(layer_count);
            VV_CHECK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()));

            // Compare found layers with requested ones.
            for (const char* layer : used_validation_layers)
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
#endif
            return true;
	    }

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
			VkSemaphoreCreateInfo semaphore_create_info = {};
			semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkSemaphore semaphore;
			VV_CHECK_SUCCESS(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphore));
			return semaphore;
		}

		static void destroyVulkanSemaphore(VkDevice device, VkSemaphore semaphore)
		{
			vkDestroySemaphore(device, semaphore, nullptr);
		}

        static VkDescriptorSetLayout createVulkanDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings)
        {
            VkDescriptorSetLayoutCreateInfo layout_create_info = {};
		    layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		    layout_create_info.bindingCount = (uint32_t)bindings.size();
		    layout_create_info.pBindings    = bindings.data();

            VkDescriptorSetLayout layout = {};
		    VV_CHECK_SUCCESS(vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &layout));
            return layout;
        }

        static VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage)
        {
            VkDescriptorSetLayoutBinding layout_binding = {};
            layout_binding.binding            = binding;
	    	layout_binding.descriptorType     = descriptor_type;
	    	layout_binding.descriptorCount    = count; // number of these elements in array sent to device
	    	layout_binding.stageFlags         = shader_stage;
	    	layout_binding.pImmutableSamplers = nullptr; // todo: figure out if I ever need this
            return layout_binding;
        }

        static VkDescriptorPool createDescriptorPool(VkDevice device)
        {
            // todo: make more general
            std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
            pool_sizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            pool_sizes[0].descriptorCount = 100;
            pool_sizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            pool_sizes[1].descriptorCount = 100; // todo: change!

            VkDescriptorPoolCreateInfo create_info = {};
            create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            create_info.poolSizeCount = (uint32_t)pool_sizes.size();
            create_info.pPoolSizes    = pool_sizes.data();
            create_info.maxSets       = 100; // todo: change!
            create_info.flags         = 0; // can be: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT

            VkDescriptorPool descriptor_pool;
            VV_CHECK_SUCCESS(vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool));
            return descriptor_pool;
        }

        static void destroyDescriptorPool(VkDevice device, VkDescriptorPool pool)
        {
            VV_ASSERT(device != VK_NULL_HANDLE, "Trying to destroy invalid descriptor pool");
            VV_ASSERT(pool != VK_NULL_HANDLE, "Trying to destroy invalid descriptor pool");

            vkDestroyDescriptorPool(device, pool, nullptr);
        }

        static VkPipelineStageFlags determinePipelineStageFlag(VkAccessFlags access_flags)
        {
            // https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#synchronization-access-types-supported
            switch (access_flags)
            {
                case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:          return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

                case VK_ACCESS_INDEX_READ_BIT:
                case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT:          return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

                case VK_ACCESS_SHADER_READ_BIT:
                case VK_ACCESS_SHADER_WRITE_BIT:
                case VK_ACCESS_UNIFORM_READ_BIT:                   return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

                case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:          return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:
                case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:          return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
                case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                                                          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

                case VK_ACCESS_TRANSFER_READ_BIT:
                case VK_ACCESS_TRANSFER_WRITE_BIT:                  return VK_PIPELINE_STAGE_TRANSFER_BIT;

                case VK_ACCESS_HOST_READ_BIT:
                case VK_ACCESS_HOST_WRITE_BIT:                      return VK_PIPELINE_STAGE_HOST_BIT;

                default:                                            return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }
        }
	}
}

namespace std
{
    template<> struct hash<vv::Vertex>
    {
        size_t operator()(vv::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

#endif // VIRTUALVISTA_UTILS_H