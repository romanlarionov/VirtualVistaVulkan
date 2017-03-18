
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define MAX_LIGHTS 5

struct Light
{
    vec4 position;
    vec4 irradiance; // radius stored in a component
};

layout(set = 0, binding = 2) uniform LightData
{
    Light lights[MAX_LIGHTS];
} lights;

layout(set = 1, binding = 0) uniform MaterialConstants
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    int shininess;
} properties;

layout(location = 0) in vec3 camera_position;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(properties.diffuse.xyz, 1.0);
}