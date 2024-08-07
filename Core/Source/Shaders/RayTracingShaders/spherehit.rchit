#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require


#include "../UtilsShaders/ShadersUtility.glsl"

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

struct Sphere {
    vec3 center;
    float radius;
    uint matId;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

layout(binding=6) uniform light{
    vec3 pos;
    vec3 col;
    float intensity;
}myLight;

layout(set = 0, binding = 7, std140) buffer Materials {
    MaterialData materials[];
};

layout(set = 0, binding = 12, scalar) buffer Spheres {
    Sphere spheres[];
};


layout(set = 0,binding = 12) uniform sampler2D textures[];

#define MAX_TEXTURES 5

vec4 CurrentMaterialTextures[MAX_TEXTURES];
int texturesOnMaterialCount = 0;

vec3 GetDiffuseColor(int materialIndex);
vec4 TryGetTex(int texIndexStart, int texOffset, vec2 uv);
float TryGetFloatFromTex(int texIndexStart, int texOffset, vec2 uv, float intensity);
vec3 GetDebugCol(uint primitiveId, float primitiveCount);
float GetLightShadingIntensity(vec3 fragPos, vec3 lightPos, vec3 normal);
void main()
{
  //materials
  rayPayload.color = vec3(0.5f, 0.5f, 0.0f);
  rayPayload.colorLit = vec3(0.5f, 0.5f, 0.0f);
  rayPayload.distance;
  rayPayload.normal = vec3(1.0f);
  rayPayload.roughness = 1.0f;
  rayPayload.reflectivity = 1.0f; 
  rayPayload.shadow = false;
  rayPayload.emissive =true;
  rayPayload.isMiss= false; 
 
}

vec4 TryGetTex(int texIndexStart, int texOffset, vec2 uv){
    if (texOffset== -1){
    
        return vec4(1, 1, 1, 1);
    }
    vec4 texture = texture(textures[texOffset],uv);
    return texture;
}

float TryGetFloatFromTex(int texIndexStart, int texOffset, vec2 uv, float intensity){
	if (texOffset== -1){
		return intensity;
	}
	vec4 texture = texture(textures[texOffset],uv);
	return texture.x * intensity;
}

vec3 GetDiffuseColor(int materialIndex){

   vec3 diffuse= materials[materialIndex].diffuseColor;
   return diffuse;
}


vec3 GetDebugCol(uint primitiveId, float primitiveCount){

    float idNormalized = float(primitiveId) / primitiveCount; 
    vec3 debugColor = vec3(idNormalized, 1.0 - idNormalized, 0.5 * idNormalized);
    return debugColor;
}
float GetLightShadingIntensity(vec3 fragPos, vec3 lightPos, vec3 normal){
    vec3 dir= fragPos-lightPos; 
    dir= normalize(dir);
    float colorShading=max(dot(dir, normal),0.2);
    return colorShading;
}

