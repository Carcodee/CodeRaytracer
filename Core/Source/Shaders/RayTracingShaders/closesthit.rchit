#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../UtilsShaders/ShadersUtility.glsl"
#include "../UtilsShaders/DisneyShadingModel.glsl"
#include "../UtilsShaders/PersonalDisney.glsl"

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

hitAttributeEXT vec2 attribs;

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

vec4 TryGetTex(int texOffset, vec2 uv);
float TryGetFloatFromTex(int texOffset, vec2 uv, float intensity);
vec3 GetDebugCol(uint primitiveId, float primitiveCount);

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
    
  vec3 bitangent = cross(tangent, normal);
  
  //todo fix transformation on shaders
  const mat3 normalTransform = mat3(gl_ObjectToWorld3x4EXT); 
  vec3 worldNormal = normalize(normalTransform * normal); 
  vec3 worldTangent = normalize(normalTransform * tangent); 
  vec3 worldBitangent = normalize(normalTransform * bitangent); 
  mat3 TBN = mat3(tangent, bitangent, normal);
  
  //materials

  int materialIndex= meshesData[realGeometryOffset].materialIndexOnShape;
  MaterialData material = materials[materialIndex];
 
  MaterialConfigurations configs;
  GetMatConfigs(material.configurations, configs); 
  
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
      finalNormal = normalize(TBN * normalInMat.xyz); 
  }

  
 material.diffuseColor = diffuseInMat; 
 material.roughnessIntensity = roughness; 
 material.metallicIntensity = metallic; 
  
 vec4 diffuse=diffuseInMat;
 vec3 view = normalize(-rayPayload.direction);
 vec3 lightDir= normalize(myLight.pos - pos); 
 vec3 halfway = normalize(view + lightDir);
 
 float cosThetaTangent = max(dot(lightDir, finalNormal), 0.001);
    
 mat3 inverseTBN = transpose(TBN);
                                        
 vec3 pbrLitDirect= GetBRDF(finalNormal * material.normalIntensity, view, lightDir, halfway, diffuse.xyz, material.baseReflection ,metallic, roughness);
 
 ////////////////////////DISNEY

 float forwardPdfW;
 float reversePdfW;
 float forwardPdfWI;
 float reversePdfWI;

 vec3 FT, FB;
 CreateOrthonormalBasis(finalNormal, FT, FB);
 
 mat3 inverseFinalTBN = transpose(mat3(FT, FB, finalNormal));

 vec3 indirectD;
 vec3 directD= EvaluateDisney(material, view, lightDir, inverseFinalTBN, configs.thin,forwardPdfW, reversePdfW);
 
 
 bool stop = true;
 int currentSample = 0;
 int maxSamples = 5;
 
  /*while(true){
      currentSample++;
      SampleDisney(rayPayload.frameSeed ,material, configs.thin, view, lightDir, inverseFinalTBN,forwardPdfWI, reversePdfWI, indirectD, stop);
      if(currentSample>maxSamples){
         break;
      }
      if(dot(indirectD, vec3(1.0f))<EPSILON || MaxComponent(indirectD) > 1.0f){
      }
  }*/
  
  SampleDisney(rayPayload.frameSeed ,material, configs.thin, view, lightDir, inverseFinalTBN,forwardPdfWI, reversePdfWI, indirectD, stop);
 
  float cosThetaTangentIndirect = max(dot(lightDir, finalNormal), 0.001);

  halfway = normalize(lightDir + view);
  vec3 pbrLitIndirect= GetBRDF(finalNormal * material.normalIntensity, view, lightDir, halfway, diffuse.xyz, material.baseReflection ,metallic, roughness); 
  
  rayPayload.shadow = true;
  float tmin = 0.001;
  float tmax = 10000.0; 
  vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT; 
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
  
  if(emissionInMat == vec4(0)){
       if(material.emissionIntensity>0){
           rayPayload.shadow = false;
           rayPayload.emissionColor = (pbrLitDirect * material.diffuseColor.xyz * material.emissionIntensity); 
       }
  }else{
       rayPayload.shadow = false;
       rayPayload.emissionColor = (emissionInMat.xyz * material.emissionIntensity); 
  }
  
  rayPayload.stop = stop;
  rayPayload.sampleDir = lightDir;
  rayPayload.hitT = gl_HitTEXT;
  rayPayload.distance = gl_RayTmaxEXT;
  rayPayload.normal = finalNormal;
  
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
vec3 GetDebugCol(uint primitiveId, float primitiveCount){
    float idNormalized = float(primitiveId) / primitiveCount; 
    vec3 debugColor = vec3(idNormalized, 1.0 - idNormalized, 0.5 * idNormalized);
    return debugColor;
}


