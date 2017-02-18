
#ifndef VIRTUALVISTA_SHADER_H
#define VIRTUALVISTA_SHADER_H

#include <vector>
#include <string>

#include "VulkanDevice.h"
#include "Resource.h"
#include "spirv_glsl.hpp"

namespace vv
{
	class Shader : public Resource
	{
	public:
		VkShaderModule vert_module = VK_NULL_HANDLE;
		VkShaderModule frag_module = VK_NULL_HANDLE;

        // specifies the binding order of the model descriptor.
        std::vector<DescriptorInfo> standard_material_descriptor_orderings;
        std::vector<DescriptorInfo> non_standard_descriptor_orderings;

		Shader();
		~Shader();

        /*
         * Manages loading of binary spir-V shader programs from file.
         *
         * todo: add support for other types of shader programs
         */
		void create(VulkanDevice *device, std::string name);

        /*
         *
         */
		void shutDown();

	private:
		VulkanDevice *_device;

		std::string _name;
		std::string _vert_path;
		std::string _frag_path;

		std::vector<char> _vert_binary_data;
		std::vector<char> _frag_binary_data;

        /*
         * Parses binary data and returns it as an array of chars.
         */
		std::vector<char> loadSpirVBinary(std::string file_name);

        /*
         * Takes in shader char array and creates a VkShaderModule from it.
         */
		void createShaderModule(const std::vector<char> byte_code, VkShaderModule &module);

        /*
         * Uses SPIRV-Cross to perform runtime reflection of the spriv shader to analyze descriptor binding info.
         */
        void reflectDescriptorTypes(std::vector<uint32_t> spirv_binar, VkShaderStageFlagBits shader_stagey);

        /*
         * Converts a descriptor type from Spirv-Cross's format to Vulkan's.
         */
        VkDescriptorType convertSpirvCrossToVulkanTypes(spirv_cross::SPIRType type) const;
	};
}

#endif // VIRTUALVISTA_SHADER_H