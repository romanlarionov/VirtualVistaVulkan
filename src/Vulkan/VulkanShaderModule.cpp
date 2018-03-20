
#include <ostream>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "VulkanShaderModule.h"
#include "Utils.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
    std::vector<uint32_t> convert(std::vector<char> buf)
    {
        std::vector<uint32_t> output(buf.size() / sizeof(uint32_t));
        std::memcpy(output.data(), buf.data(), buf.size());
        return output;
    }


	void VulkanShaderModule::create(VulkanDevice *device, std::string name, std::string stage)
	{
		_device = device;
		_filename = name;

        if (stage == "vert")
            shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
        else if (stage == "frag")
            shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        else if (stage == "comp")
            shader_stage = VK_SHADER_STAGE_COMPUTE_BIT;
        else if (stage == "tess")
            shader_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; // todo: theres also tess_evaluation_bit
        else if (stage == "geom")
            shader_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        else if (stage == "glsl")
            shader_stage = VK_SHADER_STAGE_ALL_GRAPHICS; // todo: dont know what to put here

        std::string dir = Settings::inst()->getShaderDirectory();

		_filepath = dir + name + "_" + stage + ".spv";
        _binary_data = loadSpirVBinary(_filepath);

        if (stage == "frag")
            reflectDescriptorTypes(convert(_binary_data), VK_SHADER_STAGE_FRAGMENT_BIT);

		VkShaderModuleCreateInfo shader_module_create_info = {};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.flags = 0;
		shader_module_create_info.codeSize = _binary_data.size();
		shader_module_create_info.pCode = (uint32_t *)_binary_data.data();

		VV_CHECK_SUCCESS(vkCreateShaderModule(_device->logical_device, &shader_module_create_info, nullptr, &shader_module));
	}


	void VulkanShaderModule::shutDown()
	{
		if (shader_module != VK_NULL_HANDLE)
            vkDestroyShaderModule(_device->logical_device, shader_module, nullptr);
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////// Private
	std::vector<char> VulkanShaderModule::loadSpirVBinary(std::string file_name)
	{
		std::ifstream file(file_name, std::ios::ate | std::ios::binary);
		VV_ASSERT(file.is_open(), "Vulkan Error: failed to open Spir-V file: " + file_name);

		size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		VV_ASSERT(!buffer.empty(), "Vulkan Error: Spir-V file empty: " + file_name);
		file.close();
		return buffer;
	}


    void VulkanShaderModule::reflectDescriptorTypes(std::vector<uint32_t> spirv_binary, VkShaderStageFlagBits shader_stage)
    {
        spirv_cross::CompilerGLSL glsl(spirv_binary);
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        for (auto &resource : resources.push_constant_buffers)
        {
            auto ranges = glsl.get_active_buffer_ranges(resource.id);
            for (auto &r : ranges)
            {
                VkPushConstantRange push_constant_range = {};
                push_constant_range.offset = r.offset;
                push_constant_range.size = r.range;
                push_constant_range.stageFlags = shader_stage;
                push_constant_ranges.push_back(push_constant_range);
            }
        }

        // Get all sampled uniform buffers in the shader.
        for (auto &resource : resources.uniform_buffers)
        {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);
            DescriptorInfo descriptor_info = { binding, name, shader_stage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };

            if (set == 0)
            {
                if (name != "lights")
                    throw std::runtime_error("Descriptor set 0 is reserved: " + name);
            }
            else if (set == 1)
            {
                if (name == "properties")
                    material_descriptor_orderings.push_back(descriptor_info);
                else
                    throw std::runtime_error("Non-standard descriptor found with set 1: " + name);
            }
            else if (set == 2)
                continue;
            else
                throw std::runtime_error("Descriptor with set outside of range found: " + name);
        }

        // Get all sampled images in the shader.
        for (auto &resource : resources.sampled_images)
        {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);

            DescriptorInfo descriptor_info = { binding, name, shader_stage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };

            if (set == 0)
            {
                if (name != "lights")
                    throw std::runtime_error("Descriptor set 0 is reserved: " + name);
            }
            else if (set == 1)
            {
                // if descriptor name found is a white listed material descriptor
                if (std::find(_accepted_material_descriptors.begin(), _accepted_material_descriptors.end(), name)
                              != _accepted_material_descriptors.end())
                    material_descriptor_orderings.push_back(descriptor_info);

                else
                    throw std::runtime_error("Non-standard descriptor found with set 1: " + name);
            }
            else if (set == 2)
            {
                if (name == "brdf_lut" || name == "d_irradiance_map" || name == "s_irradiance_map")
                    uses_environmental_lighting = true;
            }
                
            else
                throw std::runtime_error("Descriptor with set outside of range found: " + name);
        }

        // sort uniforms found by binding
        std::sort(material_descriptor_orderings.begin(), material_descriptor_orderings.end(),
            [](DescriptorInfo &l, DescriptorInfo &r)
            {
                return l.binding < r.binding;
            }
        );
    }
}