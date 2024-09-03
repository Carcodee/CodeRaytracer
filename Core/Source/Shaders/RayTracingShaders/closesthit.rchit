#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require


#include "../UtilsShaders/ShadersUtility.glsl"
#include "../UtilsShaders/DisneyShadingModel.glsl"


layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

hitAttributeEXT vec2 attribs;

struct TexturesFinded{
    vec4 diffuse;
    vec4 alpha;
    vec4 specular;
    vec4 bump;
    vec4 ambient;
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


layout(set = 0, binding = 7, scalar) buffer Materials {
    MaterialData materials[];
};

layout(set = 0, binding = 8, scalar) buffer MeshesData {
    MeshData meshesData[];
};

layout(set = 0, binding = 9, scalar) buffer GeometriesOffsets {
    uint geometryOffset[];
};


layout(set = 0,binding = 14) uniform sampler2D textures[];

#define MAX_TEXTURES 5

vec4 CurrentMaterialTextures[MAX_TEXTURES];
int texturesOnMaterialCount = 0;

vec3 GetDiffuseColor(int materialIndex);
vec4 TryGetTex(int texOffset, vec2 uv);
float TryGetFloatFromTex(int texOffset, vec2 uv, float intensity);
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
  pos= gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
  
  vec3 normal= barycentricCoords.x * v1.normal + barycentricCoords.y * v2.normal + barycentricCoords.z * v3.normal;
  normal = normalize(normal);
  vec3 tangent= barycentricCoords.x * v1.tangent + barycentricCoords.y * v2.tangent + barycentricCoords.z * v3.tangent;
  tangent = normalize(tangent);
  
  if (gl_HitKindEXT == gl_HitKindBackFacingTriangleEXT)
  {
    normal = -normal;
  }
    
  vec3 bitangent = cross(normal, tangent);
  
  //todo fix transformation on shaders
  const mat3 normalTransform = mat3(gl_ObjectToWorld3x4EXT); 
  vec3 worldNormal = normalize(normalTransform * normal); 
  vec3 worldTangent = normalize(normalTransform * tangent); 
  vec3 worldBitangent = normalize(normalTransform * bitangent); 
  mat3 TBN = mat3(tangent, bitangent, normal);
  
  //normal=normalize(normal);
  //tangent = normalize(tangent); 
  //vec3 bitTangent =cross(normal, tangent); 
  //mat3 TBN = mat3(tangent, bitTangent, normal); 
  
  //materials

  int materialIndex= meshesData[realGeometryOffset].materialIndexOnShape;
  MaterialData material = materials[materialIndex];
  
  vec4 diffuseInMat = TryGetTex(material.diffuseOffset, uv) * material.diffuseColor * material.albedoIntensity;
  vec4 normalInMat = TryGetTex(material.normalOffset, uv);
  vec4 emissionInMat = TryGetTex(material.emissionOffset, uv); 
  vec4 metallicRoughness = TryGetTex(material.metallicRouhgnessOffset, uv); 
  float metallic = 0.0f;
  float roughness = 1.0f;
  
  MaterialFindInfo matInfo = GetMatInfo(diffuseInMat, normalInMat);
  
  if(!matInfo.hasDiffuse){
     diffuseInMat =vec4(material.diffuseColor.xyz, 1.0) * material.albedoIntensity;
  }
  if(emissionInMat == vec4(1.0f)){
    emissionInMat = vec4(0.0f);
  }
  if(metallicRoughness == vec4(1.0f)){
    metallic =TryGetFloatFromTex(material.metallicOffset ,uv, material.metallicIntensity);
    roughness =TryGetFloatFromTex(material.roughnessOffset ,uv, material.roughnessIntensity);
  }else{
    //metallic = 0.0f;
    metallic = metallicRoughness.r * material.metallicIntensity;
    roughness = metallicRoughness.g * material.roughnessIntensity;
  }

  vec3 finalNormal = normal;
  if(matInfo.hasNormals){
      //mat3 inverseTBN = inverse(TBN);
      finalNormal = normalize(TBN * normalInMat.xyz); 
  }

  
  vec4 diffuse=diffuseInMat;
  vec3 view = normalize(-rayPayload.direction);
  vec3 lightDir= normalize(myLight.pos - pos); 
  vec3 halfway =normalize(view + lightDir);
  
  vec3 lightDirTangSpace = lightDir * TBN;
  vec3 finalNormalTangSpace = finalNormal * TBN;
  vec3 rayDirWi = normalize(pos - rayPayload.origin);
  vec3 rayDirTangentSpace = rayDirWi * TBN;
  
  float cosThetaTangent = max(dot(lightDirTangSpace, finalNormalTangSpace), 0.001);
  float cosThetaTangentIndirect = max(dot(rayPayload.sampleDir, finalNormal * TBN), 0.001);
  
    
  mat3 inverseTBN = transpose(TBN);
  vec3 hl = inverseTBN * halfway;
  vec3 wlIn = inverseTBN * lightDir ;
  vec3 wlInSample =inverseTBN * rayPayload.sampleDir;
  vec3 wlOut = inverseTBN * view ;
                                    
                                    
  vec3 pbrLitDirect= GetBRDF(finalNormal* material.normalIntensity, view, lightDir, halfway, diffuse.xyz, material.baseReflection ,metallic, roughness);
  
  halfway = normalize((-rayPayload.sampleDir) + view);
  hl = inverseTBN * halfway;

                                        
  float pdfDirect = CosinePdfHemisphere(cosThetaTangent);
  float pdf = CosinePdfHemisphere(cosThetaTangentIndirect);


  vec3 pbrLitIndirect= GetBRDF(finalNormal * material.normalIntensity, view, rayPayload.sampleDir, halfway, diffuse.xyz, material.baseReflection ,metallic, roughness);
  
  float forwardPdfD;
  float forwardPdfI;
  bool thin = true;
  vec3 disneyDirect= DisneyEval(material, view, lightDir, finalNormal,  forwardPdfD);
  float pdfI;
  vec3 sampleDir = DisneySample(material, rayPayload.frameSeed, view, finalNormal, lightDir, pdfI);
  vec3 disneyIndirect= DisneyEval(material, view, sampleDir, finalNormal, forwardPdfI);
  
  rayPayload.shadow = true;
  float tmin = 0.001;
  float tmax = 10000.0; 
  vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT; 
  traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT , 0xff, 0, 0, 1, origin, tmin, lightDir, tmax, 0);
  
  
  MaterialConfigurations configs;
  GetMatConfigs(material.configurations, configs);
  
  if(configs.useDisneyBSDF){
    rayPayload.color = disneyDirect * myLight.col  * myLight.intensity; 
    rayPayload.colorLit = disneyIndirect; 
  }else{
    rayPayload.color = pbrLitDirect * myLight.col * cosThetaTangent * myLight.intensity/ pdfDirect; 
    rayPayload.colorLit = (pbrLitIndirect * cosThetaTangentIndirect) /pdf; 
  }
  
  
  //rayPayload.color = worldNormal; 
  //rayPayload.color = vec3(diffuseInMat.a); 
  //rayPayload.colorLit = vec3(0.0f); 
  
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
  rayPayload.distance = gl_RayTmaxEXT;
  rayPayload.normal = finalNormal;
  rayPayload.tangent = tangent;
  rayPayload.roughness = material.roughnessIntensity;
  rayPayload.reflectivity = material.reflectivityIntensity;
    
  
}

vec4 TryGetTex(int texOffset, vec2 uv){
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
	vec4 textureCol = texture(textures[texOffset],uv);
	float texVal = MaxComponent(textureCol.xyz);
	return texVal * intensity;
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

