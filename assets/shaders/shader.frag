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

const float PI = 3.14159265 ;
float NdotL ;
float NdotV ;
float NdotH ;

float alpha;
float metal;

vec3 lightDirection;
vec3 viewPos;


vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0);
float GeometrySchlickGGX(float NdotV);
float DistributionGGX();
float GeometrySmith();


void main() {

    lightDirection = normalize(vec3(-0.5, 1.0, 1.0)); 
    viewPos = vec3(0.0f, 1.5f, -3.f); // Camera position in world space
    vec2 uv = UV ; 
    vec3 N = texture(normalTexture, uv).rgb;
    N = normalize(TBN * (N * 2.0 - 1.0));
   
    vec3 alb = texture(albedoTexture, uv).rgb;
    alpha = texture(roughTexture, uv).r;
    metal = texture(metalTexture, uv).r  ;    

    vec3 F0 = mix(vec3(0.4), alb, metal); 


    vec3 V = normalize(viewPos - posInWS);    // View Direction
    vec3 L = normalize(-lightDirection);      // Light Direction
    vec3 H = normalize(L + V);                // Half-way vector

    NdotL = max(dot(N, L), 0.0);              // cache these calculations
    NdotV = max(dot(N, V), 0.0);
    NdotH = max(dot(N, H), 0.0);

    /////////////////////
    // Cook-Torrance BRDF
    float D = DistributionGGX();   
    float G = GeometrySmith(); 
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
  // vec3 F = fresnelSchlickRoughness(dot(H, V), F0);

           
    vec3 numerator    = D * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;	  
    vec3 Lo = (kD * alb / PI + specular) * NdotL;  
    vec3 ambient = vec3(0.1) * alb;
    
    vec3 color = ambient + Lo;

    FragColour = vec4(color, 1.0);

}

float DistributionGGX()
{
    float a = alpha * alpha;
    //float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float denominator = (NdotH2 * (a - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return a / denominator;
}

float GeometrySchlickGGX(float Ndot)
{
    float r = (alpha + 1.0);
    float k = (r * r) / 8.0;
    float denominator = Ndot * (1.0 - k) + k;

    return Ndot / denominator;
}

float GeometrySmith()
{
    float ggx2 = GeometrySchlickGGX(NdotV);
    float ggx1 = GeometrySchlickGGX(NdotL);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0)
{
    return F0 + (max(vec3(1.0 - alpha), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  
