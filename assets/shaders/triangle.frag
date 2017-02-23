
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 1, binding = 0) uniform MaterialConstants
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    int shininess;
} constants;

layout(location = 0) in vec3 camera_position;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(constants.diffuse.xyz, 1.0);
}