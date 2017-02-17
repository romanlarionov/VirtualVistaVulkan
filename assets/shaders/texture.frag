
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//layout(set = 0, binding = 1) uniform sampler2D diffuseTex;
layout(set = 1, binding = 0) uniform MaterialConstants {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    int shininess;
} constants;

layout(set = 1, binding = 1) uniform sampler2D diffuseMap;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(diffuseMap, fragTexCoord);
    //outColor = vec4(constants.diffuse.xyz, 1.0);
}