
#ifndef VIRTUALVISTA_VULKANDESCRIPTORSETLAYOUT_H
#define VIRTUALVISTA_VULKANDESCRIPTORSETLAYOUT_H

#include <vector>

#include "VulkanDevice.h"

namespace vv
{
    struct DescriptorLayoutTypeStruct
    {
        enum LayoutType
        {
            LIGHTS = 0,
            MATERIALS = 1,
            GENERAL = 2
        };
    };

    struct DescriptorTypeStruct
    {
        enum DescriptorType
        {
            CONSTANTS = 1, // single value diffuse, ambient, specular
            AMBIENT_MAP = 2,
            DIFFUSE_MAP = 3,
            SPECULAR_MAP = 4
        };
    };

    typedef DescriptorLayoutTypeStruct::LayoutType DescriptorLayoutType;
    typedef DescriptorTypeStruct::DescriptorType DescriptorType;

	class VulkanDescriptorSetLayout
	{
	public:
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;

		VulkanDescriptorSetLayout();
		~VulkanDescriptorSetLayout();

		/*
		 * Descriptor Set Layouts serve as a template for uniform data set to shader stages.
		 * MaterialTemplates should use these in conjunction with their assigned Shaders.
		 */
		void create(VulkanDevice *device);

		/*
		 *
		 */
		void shutDown();

		/*
		 * Adds a number of descriptor bindings to the layout. Should be called prior to create()
		 * note: the bindings are ordered based on the call order. Call in order specified by shader to avoid runtime errors.
		 * note: many descriptor types: https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkDescriptorType
		 */
		void addDescriptorSetBinding(VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage);
	
	private:
		VulkanDevice *device_;

		std::vector<VkDescriptorSetLayoutBinding> bindings_;
	};
}

#endif // VIRTUALVISTA_VULKANDESCRIPTORSETLAYOUT_H