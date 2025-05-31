#version 450
layout(location = 0) out vec4 FragColour;

layout(binding = 1) uniform sampler2D albedoTexture;
layout(binding = 2) uniform sampler2D metalTexture;
layout(binding = 3) uniform sampler2D normalTexture;
layout(binding = 4) uniform sampler2D roughTexture;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 posInWS;
layout(location = 3) in mat3 TBN;

const float PI = 3.14159265;

// Pass all needed inputs to each function

float DistributionGGX(float alpha, float NdotH);
float GeometrySchlickGGX(float Ndot, float alpha);
float GeometrySmith(float NdotV, float NdotL, float alpha);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 aces(vec3 x);


void main() {
    // Setup uniforms (replace hardcoded with uniforms if possible)
    vec3 lightDirection = normalize(vec3(1.0, -10.0, 13));
    vec3 viewPos = vec3(0.0, 1.5, -3.0);

    vec2 uv = UV;
    vec3 N_sample = texture(normalTexture, uv).rgb;
    vec3 N = normalize(TBN * (N_sample * 2.0 - 1.0));

    vec3 alb = texture(albedoTexture, uv).rgb;
    float roughness = texture(roughTexture, uv).r;
    float metal = texture(metalTexture, uv).r;

    vec3 F0 = mix(vec3(0.04), alb, metal);

    vec3 V = normalize(viewPos - posInWS);    // View direction
    vec3 L = normalize(-lightDirection);      // Light direction
    vec3 H = normalize(L + V);                 // Halfway vector

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // Cook-Torrance BRDF components
    float D = DistributionGGX(roughness, NdotH);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    vec3 F = fresnelSchlick(HdotV, F0);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 Lo = (kD * alb / PI + specular) * NdotL;
    vec3 ambient = vec3(0.03) * alb;

    vec3 color = ambient + Lo;

    color = aces(color); // tone mapping
    color = pow(color, vec3(1.0 / 2)); // gamma correction

    FragColour = vec4(color, 1.0);
}

// GGX Normal Distribution function
float DistributionGGX(float alpha, float NdotH) {
    float a = alpha * alpha;
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a / denom;
}

// Schlick-GGX Geometry term for one direction
float GeometrySchlickGGX(float Ndot, float alpha) {
    float r = alpha + 1.0;
    float k = (r * r) / 8.0;
    float denom = Ndot * (1.0 - k) + k;
    return Ndot / denom;
}

// Smith Geometry function combines both view and light terms
float GeometrySmith(float NdotV, float NdotL, float alpha) {
    float ggxV = GeometrySchlickGGX(NdotV, alpha);
    float ggxL = GeometrySchlickGGX(NdotL, alpha);
    return ggxV * ggxL;
}

// Standard Fresnel Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ACES tone mapping curve (Narkowicz 2015)
vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
