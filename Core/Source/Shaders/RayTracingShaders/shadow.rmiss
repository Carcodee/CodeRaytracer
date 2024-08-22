#version 460
#extension GL_EXT_ray_tracing : enable


#include "../UtilsShaders/ShadersUtility.glsl"

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main()
{
	rayPayload.shadow = false;
	
}