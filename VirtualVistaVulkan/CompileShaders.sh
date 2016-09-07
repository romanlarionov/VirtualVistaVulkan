#!/bin/bash

Vulkan_DIR="${VulkanSDK}"
Shader_Name="$1"
${Vulkan_DIR}/Bin/glslangValidator.exe -V ${Shader_Name}.vert
${Vulkan_DIR}/Bin/glslangValidator.exe -V ${Shader_Name}.frag

mv vert.spv "${Shader_Name}_vert.spv"
mv frag.spv "${Shader_Name}_frag.spv"
