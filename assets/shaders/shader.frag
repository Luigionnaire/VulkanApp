#version 450
layout(location = 0) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D metallic;
layout(binding = 3) uniform sampler2D normal;
layout(binding = 4) uniform sampler2D roughness;


layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(roughness, fragTexCoord);
}