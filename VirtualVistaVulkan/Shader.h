
#ifndef VIRTUALVISTA_SHADER_H
#define VIRTUALVISTA_SHADER_H

#include <vector>
#include <string>

#include "VulkanDevice.h"
#include "Resource.h"

namespace vv
{
	class Shader : public Resource
	{
	public:
		VkShaderModule vert_module = VK_NULL_HANDLE;
		VkShaderModule frag_module = VK_NULL_HANDLE;

		Shader();
		~Shader();

        /*
         * Manages loading of binary spir-V shader programs from file.
         * todo: dynamically construct descriptor set layouts through runtime shader
         *       parsing using library: https://github.com/KhronosGroup/SPIRV-Cross
         * todo: add support for other types of shader programs
         */
		void create(VulkanDevice *device, std::string name);

        /*
         *
         */
		void shutDown();

	private:
		VulkanDevice *device_;

		std::string name_;
		std::string vert_path_;
		std::string frag_path_;

		std::vector<char> vert_binary_data_;
		std::vector<char> frag_binary_data_;

        /*
         * Parses binary data and returns it as an array of chars.
         */
		std::vector<char> loadSpirVBinary(std::string file_name);

        /*
         * Takes in shader char array and creates a VkShaderModule from it.
         */
		void createShaderModule(const std::vector<char> byte_code, VkShaderModule &module);
	};
}

#endif // VIRTUALVISTA_SHADER_H