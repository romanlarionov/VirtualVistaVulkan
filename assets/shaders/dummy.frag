
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 camera_position;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(1.0, 1.0, 1.0, 1.0);
}
