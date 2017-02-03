
#include "VulkanDescriptorSetLayout.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout()
	{
	}


	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
	}


	void VulkanDescriptorSetLayout::create(VulkanDevice *device)
	{
		device_ = device;

		if (bindings_.empty()) return;

		VkDescriptorSetLayoutCreateInfo layout_create_info = {};
		layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_create_info.bindingCount = (uint32_t)bindings_.size();
		layout_create_info.pBindings = bindings_.data();

		VV_CHECK_SUCCESS(vkCreateDescriptorSetLayout(device->logical_device, &layout_create_info, nullptr, &layout));
	}


	void VulkanDescriptorSetLayout::shutDown()
	{
		vkDestroyDescriptorSetLayout(device_->logical_device, layout, nullptr);
	}


	void VulkanDescriptorSetLayout::addDescriptorSetBinding(VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage)
	{
		VkDescriptorSetLayoutBinding layout_binding = {};
		layout_binding.binding = (uint32_t)bindings_.size(); // todo: test
		layout_binding.descriptorType = descriptor_type;
		layout_binding.descriptorCount = count; // number of these elements in array sent to device

		layout_binding.stageFlags = shader_stage;
		layout_binding.pImmutableSamplers = nullptr; // todo: figure out if I ever need this
		bindings_.push_back(layout_binding);
	}


	///////////////////////////////////////////////////////////////////////////////////////////// Private
}
