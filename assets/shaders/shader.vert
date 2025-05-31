#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;

layout(location = 0) out vec2 UV;
layout(location = 1) out vec3 norm;
layout(location = 2) out vec3 posInWS;
layout(location = 3) out mat3 TBN;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
//    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(tangent , 1.0);
//    fragTexCoord = texCoord;
//
    posInWS = (ubo.model * vec4(pos, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * vec4(posInWS, 1.0);
    norm = (ubo.model * vec4(normal, 0.0)).xyz;
    UV = texCoord;
    vec3 T = (ubo.model * vec4(tangent, 0.0)).xyz;
    vec3 B = cross(norm, T);
    TBN = mat3(T, B, norm);

}