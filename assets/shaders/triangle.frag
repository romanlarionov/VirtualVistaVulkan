
#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(set = 0, binding = 1) uniform sampler2D diffuseTex;
/*layout(set = 1, binding = 0) uniform MaterialConstants {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    int shininess;
} constants;*/

//layout(location = 0) in vec3 fragColor;
//layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = texture(diffuseTex, fragTexCoord);
    //outColor = vec4(constants.diffuse, 1.0);
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
}