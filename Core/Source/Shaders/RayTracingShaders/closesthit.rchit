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


layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

hitAttributeEXT vec2 attribs;

struct TexturesFinded{
    vec4 diffuse;
    vec4 alpha;
    vec4 specular;
    vec4 bump;
    vec4 ambient;
};

struct MeshData {
    int materialIndexOnShape;
    int geometryIndexStartOffset;
    int indexOffset;
};


struct Vertex {
    vec3 position; 
    vec3 col;
    vec3 normal;
    vec3 tangent;
    vec2 texCoords; 
};

struct Indices {
    uint index;
};

layout(binding=6) uniform light{
    vec3 pos;
    vec3 col;
    float intensity;
}myLight;


layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;

layout(set=0, binding=3) uniform sampler2D mySampler;


layout(set = 0, binding = 4, scalar) buffer VertexBuffer {
    Vertex vertices[];
};

layout(set = 0, binding = 5, scalar) buffer IndexBuffer {
    Indices indices[];
};


layout(set = 0, binding = 7, std140) buffer Materials {
    MaterialData materials[];
};

layout(set = 0, binding = 8, scalar) buffer MeshesData {
    MeshData meshesData[];
};

layout(set = 0, binding = 9, scalar) buffer GeometriesOffsets {
    uint geometryOffset[];
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
    
  int realGeometryOffset= int(geometryOffset[gl_InstanceID]) + gl_GeometryIndexEXT; 

  int primitiveIndex=int(meshesData[realGeometryOffset].geometryIndexStartOffset);
  
  int indexOffset=int(meshesData[realGeometryOffset].indexOffset);

  int idx1= primitiveIndex + (3 * gl_PrimitiveID + 0);
  int idx2= primitiveIndex + (3 * gl_PrimitiveID + 1);
  int idx3= primitiveIndex + (3 * gl_PrimitiveID + 2);
  
  int index1= indexOffset + int(indices[idx1].index);
  int index2= indexOffset + int(indices[idx2].index);
  int index3= indexOffset + int(indices[idx3].index);

  Vertex v1 = vertices[index1];
  Vertex v2 = vertices[index2];
  Vertex v3 = vertices[index3];
  
  const vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  vec2 uv = barycentricCoords.x * v1.texCoords + barycentricCoords.y * v2.texCoords + barycentricCoords.z * v3.texCoords;
  vec3 pos= barycentricCoords.x * v1.position + barycentricCoords.y * v2.position + barycentricCoords.z * v3.position;
  vec3 normal= barycentricCoords.x * v1.normal + barycentricCoords.y * v2.normal + barycentricCoords.z * v3.normal;
  vec3 tangent= barycentricCoords.x * v1.tangent + barycentricCoords.y * v2.tangent + barycentricCoords.z * v3.tangent;
  
  normal=normalize(normal);
  tangent=normalize(tangent);
  vec3 bitTangent =cross(normal, tangent); 
  mat3 TBN = mat3(tangent, bitTangent, normal);

  //materials

  int materialIndex= meshesData[realGeometryOffset].materialIndexOnShape;
  int materialIndexInTextures=materials[materialIndex].texturesIndexStart;
  int materialTextureSizes =materials[materialIndex].textureSizes;
  
  vec4 diffuseInMat = TryGetTex(materialIndexInTextures, materials[materialIndex].diffuseOffset, uv) * materials[materialIndex].albedoIntensity;
  vec4 normalInMat = TryGetTex(materialIndexInTextures, materials[materialIndex].normalOffset, uv);
  MaterialFindInfo matInfo = GetMatInfo(diffuseInMat, normalInMat);
  
  if(!matInfo.hasDiffuse){
     diffuseInMat =vec4(materials[materialIndex].diffuseColor, 1.0);
  }
  vec3 finalNormal = normal;
  if(matInfo.hasNormals){
      mat3 inverseTBN = transpose(TBN);
      vec3 normalWorldSpace = normalInMat.xyz * inverseTBN;
      finalNormal = normalize(normalWorldSpace) * materials[materialIndex].normalIntensity; 
  }
  
  
  vec4 diffuse=diffuseInMat;

  vec3 view = normalize(-rayPayload.direction) * TBN;
  vec3 lightDir= normalize(myLight.pos - pos); 
  vec3 halfway =normalize(view + lightDir);
  
  vec3 lightDirTangSpace = lightDir * TBN;
  vec3 finalNormalTangSpace = finalNormal * TBN;
  vec3 rayDirWi = normalize(pos - rayPayload.origin);
  vec3 rayDirTangentSpace = rayDirWi * TBN;
  
  float cosThetaTangent = max(dot(lightDirTangSpace, finalNormalTangSpace), 0.001);
  float cosThetaTangentIndirect = max(dot(rayPayload.sampleDir, finalNormal * TBN), 0.001);
  
  float roughness =TryGetFloatFromTex(materialIndexInTextures, materials[materialIndex].roughnessOffset ,uv, materials[materialIndex].roughnessIntensity);
  float metallic =TryGetFloatFromTex(materialIndexInTextures, materials[materialIndex].metallicOffset ,uv, materials[materialIndex].metallicIntensity);
  
  
  vec3 pbrLitDirect= GetBRDF(finalNormal, view, lightDir, halfway, diffuse.xyz, materials[materialIndex].baseReflection ,metallic, roughness);
  
  halfway = normalize(rayPayload.sampleDir + view);
    
  float pdf = CosinePdfHemisphere(cosThetaTangentIndirect);
  vec3 pbrLitIndirect= GetBRDF(finalNormal, view, rayPayload.sampleDir, halfway, diffuse.xyz, materials[materialIndex].baseReflection ,metallic, roughness);
  
  rayPayload.shadow = true;
  float tmin = 0.001;
  float tmax = 10000.0; 
  vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT; 
  traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT , 0xff, 0, 0, 1, origin, tmin, lightDir, tmax, 0);
  rayPayload.color = materials[materialIndex].emissionIntensity + (pbrLitDirect * cosThetaTangent * myLight.intensity * myLight.col); 
  rayPayload.colorLit = (pbrLitIndirect * cosThetaTangentIndirect) /pdf; 
  
  if(materials[materialIndex].emissionIntensity>0){
       rayPayload.emissive = true;
       rayPayload.shadow = false;
       rayPayload.color = pbrLitDirect * materials[materialIndex].diffuseColor * materials[materialIndex].emissionIntensity; 
  }
 
  rayPayload.distance = gl_RayTmaxEXT;
  rayPayload.normal = normal;
  rayPayload.roughness = materials[materialIndex].roughnessIntensity;
  rayPayload.reflectivity = materials[materialIndex].reflectivityIntensity;
  
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

