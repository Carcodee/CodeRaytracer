#version 460
#extension GL_EXT_ray_tracing : enable

struct RayPayload{
    vec3 color;
    vec3 colorLit;
    float distance;
    vec3 normal;
    vec3 origin;
    vec3 direction;
    vec3 sampleDir;
    float roughness;
    float reflectivity;
    bool shadow;
    bool emissive;
    bool isMiss;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

layout(set = 0, binding = 11) uniform sampler2D environmentMap;

void main()
{
    const vec3 gradientStart = vec3(0.5, 0.6, 1.0);
    const vec3 gradientEnd = vec3(1.0);
    vec3 unitDir = normalize(gl_WorldRayDirectionEXT);
    // Compute spherical coordinates
    float theta = acos(unitDir.y);
    float phi = atan(unitDir.z, unitDir.x) + 3.1415f; // Add PI to shift the range [0, 2*PI]
    
    // Normalize spherical coordinates to [0, 1] range for texture lookup
    
    float u = phi / (2.0 * 3.1415f);
    float v = theta / 3.1415;
    float t = 0.5 * (unitDir.y + 1.0);
    
    vec3 gradientCol = (1.0-t) * gradientStart + t * gradientEnd;
    vec3 envMap=texture(environmentMap, vec2 (u,v)).rgb;
    
	rayPayload.color = envMap;
	rayPayload.isMiss = true;
	rayPayload.distance = -1.0f;
	
}