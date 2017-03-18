
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define MAX_LIGHTS 5
#define ONE_OVER_PI 0.3183098861837906715377675267450

struct Light
{
    vec4 position;
    vec4 irradiance; // radius stored in a component
};

layout(set = 0, binding = 2) uniform LightData
{
    Light lights[MAX_LIGHTS];
} lights;

layout(push_constant) uniform PushConstants
{
    uint total_mip_levels;
} constants;

layout (set = 1, binding = 0) uniform sampler2D albedo_map;
layout (set = 1, binding = 1) uniform sampler2D roughness_map;
layout (set = 1, binding = 2) uniform sampler2D metalness_map;

layout (set = 2, binding = 0) uniform samplerCube d_irradiance_map;
layout (set = 2, binding = 1) uniform samplerCube s_irradiance_map;
layout (set = 2, binding = 2) uniform sampler2D brdf_lut;

layout(location = 0) in vec3 w_frag_position;
layout(location = 1) in vec3 w_cam_position;
layout(location = 2) in vec3 in_w_normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 out_color;

vec3 contributeAnalytic(vec3 w_light_position, vec3 irradiance, float radius)
{
    vec3 w_light_dir = w_light_position - w_frag_position;
    float dist = max(length(w_light_dir), 0.0001);
    float attenuation = radius / (dist * dist);
    return irradiance * attenuation;
}

void main()
{
    vec3 albedo = pow(texture(albedo_map, uv).rgb, vec3(2.2));
    vec3 w_normal = normalize(in_w_normal);
    float roughness = clamp(texture(roughness_map, uv).g, 0.0, 1.0);
    float metalness = clamp(texture(metalness_map, uv).r, 0.0, 1.0);

    vec3 w_view = normalize(w_cam_position - w_frag_position);
    vec3 w_reflection = normalize(reflect(-w_view, w_normal));

    float NdotV = clamp(dot(w_normal, w_view), 0.0, 1.0);
    vec2 s_brdf = textureLod(brdf_lut, vec2(NdotV, clamp(roughness, 0.0, 1.0)), 0).rg;

    // To have energy conservation, diffuse + specular brdf must be <= 1
    vec3 Kd = albedo * (1.0 - metalness) * ONE_OVER_PI;
    vec3 Ed = textureLod(d_irradiance_map, w_normal, 0).rgb;
    
    // interpolate incident fresnel by metalness %
    vec3 F0 = mix(vec3(0.04), albedo, metalness); 
    float x = constants.total_mip_levels;
    float specular_mip_level = roughness * roughness * float(constants.total_mip_levels - 1);
    vec3 Es = textureLod(s_irradiance_map, w_reflection, specular_mip_level).rgb;

    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        Light l = lights.lights[i];
        vec3 Ei = contributeAnalytic(l.position.xyz, l.irradiance.xyz, l.irradiance.a);
        Ed += Ei;
        Es += Ei;
    }

    vec3 Lo = (Ed * Kd) + (Es * (F0 * s_brdf.x + s_brdf.y));
    out_color = vec4(pow(Lo, vec3(1.0 / 2.2)), 1.0); // apply gamma correction
}