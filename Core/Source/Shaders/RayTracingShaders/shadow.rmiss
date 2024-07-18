#version 460
#extension GL_EXT_ray_tracing : enable

struct RayPayload{
    vec3 color;
    float distance;
    vec3 normal;
    vec3 tangent;
    vec3 origin;
    float roughness;
    float reflectivity;
    float directLightIntensity;
    bool shadow;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main()
{
	rayPayload.shadow = false;
}