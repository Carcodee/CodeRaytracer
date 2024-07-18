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
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main()
{

	const vec3 gradientStart = vec3(0.5, 0.6, 1.0);
	const vec3 gradientEnd = vec3(1.0);
	vec3 unitDir = normalize(gl_WorldRayDirectionEXT);
	float t = 0.5 * (unitDir.y + 1.0);
	rayPayload.color = (1.0-t) * gradientStart + t * gradientEnd;
	rayPayload.distance = -1.0f;
	rayPayload.normal = vec3(0.0f);
	rayPayload.tangent = vec3(0.0f);
	rayPayload.directLightIntensity = 1.0f;
	
}