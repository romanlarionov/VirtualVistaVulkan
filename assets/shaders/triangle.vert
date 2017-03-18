
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform SceneUBO 
{
    mat4 view;
    mat4 projection;
    vec4 camera_position;
} scene_ubo;

layout(set = 0, binding = 1) uniform ModelUBO
{
    mat4 model;
    mat4 normal;
} model_ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

layout(location = 0) out vec3 camera_position;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = scene_ubo.projection * scene_ubo.view * model_ubo.model * vec4(position, 1.0);
    camera_position = vec3(scene_ubo.camera_position);
}