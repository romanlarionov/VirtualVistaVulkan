
#ifndef VIRTUALVISTA_SHADER_H
#define VIRTUALVISTA_SHADER_H

#include <vector>
#include <string>
#include <vulkan\vulkan.h>

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

		void create(std::string path, std::string name, VkDevice device);

	private:
		VkDevice device_;

		std::string vert_path_;
		std::string frag_path_;

		std::vector<char> vert_binary_data_;
		std::vector<char> frag_binary_data_;

		std::vector<char> loadSpirVBinary(std::string file_name);
		void createShaderModule(const std::vector<char> byte_code, VkShaderModule &module);
	};
}

#endif // VIRTUALVISTA_SHADER_H