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

    posInWS = (ubo.model * vec4(pos, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * vec4(posInWS, 1.0);

    // Transform normal and tangent with normalMatrix
    vec3 T = normalize(vec3(ubo.model * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(ubo.model * vec4(normal, 0.0)));

    T = normalize(T - dot(T, N) * N); // Ensure T is orthogonal to N
    vec3 B = normalize(cross(N, T));

    norm = N;
    UV = texCoord;
    TBN = mat3(T, B, N);

}