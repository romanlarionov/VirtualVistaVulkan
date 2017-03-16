
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

layout(location = 0) out vec3 uvw;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    mat3 scale = mat3(vec3(20.0, 0.0, 0.0), vec3(0.0, 20.0, 0.0), vec3(0.0, 0.0, 20.0));

    gl_Position = ubo.projection * ubo.view * vec4(scale * position, 1.0);
    uvw = scale * position;
}