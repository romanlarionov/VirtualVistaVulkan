
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

layout(set = 1, binding = 1) uniform sampler2D diffuse_map;
layout(set = 1, binding = 2) uniform sampler2D specular_map;

layout (location = 0) in vec3 frag_position;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 camera_position;

layout(location = 0) out vec4 out_color;

vec3 blinnPhongShade(vec3 p, vec3 n, vec3 pv, vec3 Kd, vec3 Ks, float shininess, vec3 lp, vec3 Er, float lr)
{
    vec3 v = normalize(pv - p);
    vec3 l = lp - p;
    float dist = length(l);
    l = normalize(l);

    vec3 r = reflect(-l, n);
    float RdotV = clamp(dot(r, v), 0.0, 1.0);
    float atten = lr / (dist * dist + 1.0);

    return (Kd + Ks * pow(RdotV, shininess)) * Er * atten;
}

void main()
{
    vec3 Lo = vec3(0.0);
    vec4 Kd = texture(diffuse_map, tex_coord);
    vec4 Ks = texture(specular_map, tex_coord);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        Light l = lights.lights[i];

        Lo += blinnPhongShade(frag_position.xyz, normalize(normal.xyz), camera_position.xyz,
              Kd.xyz, Ks.xyz, properties.shininess,
              l.position.xyz, l.irradiance.xyz, l.irradiance.a);
    }

    out_color = vec4(Lo, 1.0);
}