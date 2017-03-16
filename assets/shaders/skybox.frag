
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

/*#define MAX_LIGHTS 5

struct Light
{
    vec4 position;
    vec4 irradiance;
};

layout(set = 0, binding = 1) uniform LightData
{
    Light lights[MAX_LIGHTS];
} lights;*/

layout(set = 1, binding = 0) uniform samplerCube radiance_map;

layout(location = 0) in vec3 uvw;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(textureLod(radiance_map, uvw, 0).xyz, 0.0);
    //out_color = vec4(1.0);
}
