
#ifndef VIRTUALVISTA_VULKANSHADERMODULE_H
#define VIRTUALVISTA_VULKANSHADERMODULE_H

#include <vector>
#include <string>

#include "VulkanDevice.h"
#include "spirv_glsl.hpp"

namespace vv
{
    class VulkanShaderModule
    {
    public:
        VkShaderModule shader_module = VK_NULL_HANDLE;
        VkShaderStageFlagBits shader_stage;
        std::string entrance_function;

        // specifies the binding order of the model descriptor.
        std::vector<DescriptorInfo> material_descriptor_orderings;
        std::vector<VkPushConstantRange> push_constant_ranges;
        bool uses_environmental_lighting = false;

        VulkanShaderModule() = default;
        ~VulkanShaderModule() = default;
        VulkanShaderModule(const VulkanShaderModule&) = default;
        VulkanShaderModule& operator=(const VulkanShaderModule&) = default;

        /*
         * Manages loading of binary Spir-V shader programs from file.
         */
        void create(VulkanDevice *device, std::string name, std::string stage, std::string entrance_function);

        /*
         *
         */
        void shutDown();

    private:
        VulkanDevice *_device;

        std::string _filename;
        std::string _filepath;
        std::vector<char> _binary_data;

        std::vector<const char *> _accepted_material_descriptors =
        {
            "ambient_map",
            "diffuse_map",
            "specular_map",
            "normal_map",
            "roughness_map",
            "metalness_map",
            "albedo_map",
            "emissiveness_map",
            "ambient_occlusion_map",
            "radiance_map"
        };

        /*
         * Parses binary data and returns it as an array of chars.
         */
        std::vector<char> loadSpirVBinary(std::string file_name);

        /*
         * Uses SPIRV-Cross to perform runtime reflection of the spriv shader to analyze descriptor binding info.
         */
        void reflectDescriptorTypes(std::vector<uint32_t> spirv_binary, VkShaderStageFlagBits shader_stage);
    };
}

#endif // VIRTUALVISTA_VULKANSHADERMODULE_H