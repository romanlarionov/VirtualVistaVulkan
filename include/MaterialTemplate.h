
#ifndef VIRTUALVISTA_MATERIALTEMPLATE_H
#define VIRTUALVISTA_MATERIALTEMPLATE_H

#include <vector>
#include <utility>

#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "Shader.h"
#include "VulkanRenderPass.h"

namespace vv
{
    struct MaterialTemplate
    {
        std::string name;
        VkPipelineLayout pipeline_layout;
        VulkanPipeline *pipeline;
        Shader *shader;
        VkDescriptorSetLayout material_descriptor_set_layout;
        bool uses_environment_lighting;
    };
}

#endif // VIRTUALVISTA_MATERIALTEMPLATE_H