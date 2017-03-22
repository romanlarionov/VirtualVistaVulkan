#!/bin/bash

# usage example:
# ./CompileShaders.sh skybox


Shader_Name="$1"
${VULKAN_SDK}/Bin/glslangValidator.exe -V ${Shader_Name}.vert
${VULKAN_SDK}/Bin/glslangValidator.exe -V ${Shader_Name}.frag

mv vert.spv "${Shader_Name}_vert.spv"
mv frag.spv "${Shader_Name}_frag.spv"
