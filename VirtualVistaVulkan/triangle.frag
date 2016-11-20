
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D diffuseTex;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 col = vec4(0.5);
    vec4 diffuse_color = texture(diffuseTex, fragTexCoord);
    outColor = diffuse_color;//vec4(fragTexCoord, 0.0, 1.0);
}