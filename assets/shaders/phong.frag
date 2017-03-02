
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define MAX_LIGHTS 5

struct Light
{
    vec4 position;
    vec4 irradiance;
    float radius;
    vec3 pad;
};

layout(set = 0, binding = 1) uniform LightData
{
    Light lights[MAX_LIGHTS];
} lights;

layout(set = 1, binding = 0) uniform MaterialConstants
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    int shininess;
} constants;

//layout(set = 1, binding = 1) uniform sampler2D ambient_texture;
layout(set = 1, binding = 1) uniform sampler2D diffuse_texture;
layout(set = 1, binding = 2) uniform sampler2D specular_texture;

layout (location = 0) in vec3 frag_position;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 camera_position;

layout(location = 0) out vec4 out_color;

void main()
{
    vec3 Normal = normalize(normal);
    vec3 view = normalize(frag_position - camera_position);

    vec4 Lo = vec4(0.0);
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        vec4 Kd = texture(diffuse_texture, tex_coord);
        vec4 Ks = texture(specular_texture, tex_coord);
        vec3 light = -normalize(lights.lights[i].position.xyz - frag_position);
        float dist = length(light);
        float attenuation = lights.lights[i].radius / (dist * dist + 1.0);

        vec3 halfway = normalize(view + light);
        vec3 reflection = normalize(reflect(-light, Normal));
        float RdotV = max(dot(reflection, view), 0.0);
        float NdotH = dot(Normal, halfway);
        float NdotL = dot(Normal, light);
        Lo += (Kd +  (pow(RdotV, constants.shininess) * Ks)) * lights.lights[i].irradiance * NdotL * attenuation;
    }

    out_color = vec4(Lo.xyz, 1.0);
}