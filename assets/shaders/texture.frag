
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

layout(set = 1, binding = 1) uniform sampler2D ambient_texture;
layout(set = 1, binding = 2) uniform sampler2D diffuse_texture;

layout(location = 0) in vec3 camera_position;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = texture(diffuse_texture, tex_coord);
}