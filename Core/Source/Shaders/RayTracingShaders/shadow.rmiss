#version 460
#extension GL_EXT_ray_tracing : enable

struct RayPayload{
    vec3 color;
    vec3 colorLit;
    vec3 emissionColor;
    float distance;
    vec3 normal;
    vec3 origin;
    vec3 direction;
    vec3 sampleDir;
    float roughness;
    float reflectivity; 
    bool shadow;
    bool isMiss;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main()
{
	rayPayload.shadow = false;
	
}