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


layout(set = 0,binding = 13) uniform sampler2D textures[];

#define MAX_TEXTURES 5

vec4 CurrentMaterialTextures[MAX_TEXTURES];
int texturesOnMaterialCount = 0;

vec3 GetDiffuseColor(int materialIndex);
vec4 TryGetTex(int texOffset, vec2 uv);
float TryGetFloatFromTex( int texOffset, vec2 uv, float intensity);
vec3 GetDebugCol(uint primitiveId, float primitiveCount);
float GetLightShadingIntensity(vec3 fragPos, vec3 lightPos, vec3 normal);
void main()
{

  Sphere sphere = spheres[gl_InstanceID];
  
  MaterialData material = materials[sphere.matId];
  vec3 hitPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT; 
  
  vec3 lightDir= normalize(myLight.pos - hitPos); 
  vec3 normal = normalize(hitPos - sphere.center);
  vec3 view = normalize(-rayPayload.direction);
  vec3 halfway =normalize(view + lightDir);
  vec3 rayDirWi = normalize(hitPos - rayPayload.origin);
  
  float cosThetaTangent = max(dot(lightDir, normal), 0.001);

  
  vec4 diffuseInMat = TryGetTex(material.diffuseOffset,  gl_LaunchIDEXT.xy ) * material.albedoIntensity;
  float roughness =TryGetFloatFromTex(material.roughnessOffset , gl_LaunchIDEXT.xy, material.roughnessIntensity);
  float metallic =TryGetFloatFromTex(material.metallicOffset , gl_LaunchIDEXT.xy, material.metallicIntensity);
  
  MaterialFindInfo matInfo = GetMatInfo(diffuseInMat, vec4(1));
  
  if(!matInfo.hasDiffuse){
    diffuseInMat.xyz = material.diffuseColor;
  }
  
  vec3 pbrLitDirect= GetBRDF(normal, view, lightDir, halfway, diffuseInMat.xyz, material.baseReflection ,metallic, roughness);
  
  halfway = normalize(rayPayload.sampleDir + view);
  
  vec3 pbrLitIndirect= GetBRDF(normal, view, rayPayload.sampleDir, halfway, diffuseInMat.xyz, material.baseReflection ,metallic, roughness);
  
  
  rayPayload.color = material.emissionIntensity + (pbrLitDirect * cosThetaTangent * myLight.intensity * myLight.col); 
  rayPayload.colorLit = pbrLitIndirect; 
  rayPayload.normal = normal;
  rayPayload.roughness = roughness;
  rayPayload.reflectivity = material.reflectivityIntensity; 
  rayPayload.shadow = false;
  if(material.emissionIntensity>0.0f){
    rayPayload.emissive = true;
  }else{
    rayPayload.emissive = false;
  }
  rayPayload.isMiss= false; 
 
}

vec4 TryGetTex( int texOffset, vec2 uv){
    if (texOffset== -1){
    
        return vec4(1, 1, 1, 1);
    }
    vec4 texture = texture(textures[texOffset],uv);
    return texture;
}

float TryGetFloatFromTex(int texOffset, vec2 uv, float intensity){
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

