
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 camera_position;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tex_coord;

layout(location = 0) out vec3 camera_position;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(position, 1.0);
    camera_position = vec3(ubo.camera_position);
}
