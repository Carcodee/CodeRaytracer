#version 460
#extension GL_EXT_ray_tracing : enable

struct RayPayload{
    vec3 color;
    vec3 colorLit;
    float distance;
    vec3 normal;
    vec3 origin;
    vec3 direction;
    float roughness;
    float reflectivity;
    bool shadow;
    bool emissive;
    bool isMiss;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main()
{
	rayPayload.shadow = false;
}