#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require


#include "../UtilsShaders/ShadersUtility.glsl"
#include "../UtilsShaders/DisneyShadingModel.glsl"
#include "../UtilsShaders/PersonalDisney.glsl"

struct Sphere {
    vec3 center;
    float radius;
    uint matId;
    uint instancesOffset;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;

layout(binding=6) uniform light{
    vec3 pos;
    vec3 col;
    float intensity;
}myLight;

layout(set = 0, binding = 7, scalar) buffer Materials {
    MaterialData materials[];
};

layout(set = 0, binding = 12, scalar) buffer Spheres {
    Sphere spheres[];
};


layout(set = 0,binding = 14) uniform sampler2D textures[];

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

  uint instancesOffset = spheres[0].instancesOffset;
  Sphere sphere = spheres[gl_InstanceID - instancesOffset];
  
  MaterialData material = materials[sphere.matId];
  vec3 hitPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT; 
  
  MaterialConfigurations configs;
  GetMatConfigs(material.configurations, configs); 
  
  vec3 normal = normalize(hitPos - sphere.center);
  
  vec3 T;
  vec3 B;
  CreateOrthonormalBasis(normal, T, B);
  mat3 inverseTBN = transpose(mat3(T, B, normal));
  vec3 view = normalize(-rayPayload.direction);
  vec3 lightDir= normalize(myLight.pos - hitPos); 
  vec3 halfway =normalize(view + lightDir);
  vec3 rayDirWi = normalize(hitPos - rayPayload.origin);
  
  float cosThetaTangent = max(dot(lightDir, normal), 0.001);

  
  vec4 diffuseInMat = TryGetTex(material.diffuseOffset,  gl_LaunchIDEXT.xy ) * material.albedoIntensity;
  vec4 emissionInMat = TryGetTex(material.emissionOffset, gl_LaunchIDEXT.xy); 
  float roughness =TryGetFloatFromTex(material.roughnessOffset , gl_LaunchIDEXT.xy, material.roughnessIntensity);
  float metallic =TryGetFloatFromTex(material.metallicOffset , gl_LaunchIDEXT.xy, material.metallicIntensity);
  
  MaterialFindInfo matInfo = GetMatInfo(diffuseInMat, vec4(1));
  
  if(!matInfo.hasDiffuse){
    diffuseInMat.xyz = material.diffuseColor.xyz;
  }
  if(emissionInMat == vec4(1)){
    emissionInMat = vec4(0.0f);
  }
  
  vec3 hl = inverseTBN * halfway;
  vec3 wlIn = inverseTBN * lightDir;
  vec3 wlOut = inverseTBN * view;
  
  material.diffuseColor = diffuseInMat;
  material.roughnessIntensity = roughness;
  material.metallicIntensity = metallic;
  
  float forwardPdfD;
  float forwardPdfI;
  bool thin = true;
  
  vec3 pbrLitDirect= GetBRDF(normal, view, lightDir, halfway, diffuseInMat.xyz, material.baseReflection ,metallic, roughness);
  
  float forwardPdfW;
  float reversePdfW;
  
  float forwardPdfWI;
  float reversePdfWI;
  vec3 indirectD;
  
  vec3 directD= EvaluateDisney(material, view, lightDir, inverseTBN, configs.thin,forwardPdfW, reversePdfW);
  bool stop = true;
  int maxSamples = 5;
  int currentSample = 0;
  
  while(stop){
      SampleDisney(rayPayload.frameSeed ,material, configs.thin, view, lightDir, inverseTBN,forwardPdfWI, reversePdfWI, indirectD, stop);
      currentSample++;
      if(currentSample>maxSamples){
         break;
      }
  }
                                   
  float cosThetaTangentIndirect = max(dot(lightDir, normal), 0.001);
  
  halfway = normalize(lightDir + view);
  vec3 pbrLitIndirect= GetBRDF(normal, view, lightDir, halfway, diffuseInMat.xyz, material.baseReflection ,metallic, roughness);
  
  rayPayload.shadow = true;
  float tmin = 0.001;
  float tmax = 10000.0; 
  vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT; 
  rayPayload.shadow = false;
  traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT , 0xff, 0, 0, 1, origin, tmin, lightDir, tmax, 0);
   
   
  float weight =powerHeuristic(forwardPdfW * myLight.intensity, forwardPdfWI);
  
  if(configs.useDisneyBSDF){
    rayPayload.color = directD * cosThetaTangent * myLight.col  * myLight.intensity; 
    rayPayload.colorLit = indirectD * cosThetaTangentIndirect;
    rayPayload.pdf =Lerp(forwardPdfWI, forwardPdfW, weight);
  }else{
    rayPayload.color = pbrLitDirect * myLight.col * cosThetaTangent * myLight.intensity; 
    rayPayload.colorLit = pbrLitIndirect *  cosThetaTangentIndirect; 
    rayPayload.pdf =Lerp(forwardPdfWI, forwardPdfW, weight);
  }
  
  rayPayload.sampleDir = lightDir; 
  rayPayload.normal = normal;
  rayPayload.distance = gl_RayTmaxEXT;
  rayPayload.stop = stop;
  rayPayload.pdf = forwardPdfWI;
  
  if(emissionInMat == vec4(0)){
    if(material.emissionIntensity>0){
        rayPayload.shadow = false;
        rayPayload.emissionColor = (pbrLitDirect * material.diffuseColor.xyz * material.emissionIntensity); 
    }
  }else{
       rayPayload.shadow = false;
       rayPayload.emissionColor = (emissionInMat.xyz * material.emissionIntensity); 
  }
  rayPayload.hitT = gl_HitTEXT;
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

   vec3 diffuse= materials[materialIndex].diffuseColor.xyz;
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

