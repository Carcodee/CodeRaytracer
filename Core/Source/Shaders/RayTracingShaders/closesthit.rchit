#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require


#include "../UtilsShaders/ShadersUtility.glsl"

struct RayPayload{
    vec3 color;
    float distance;
    vec3 normal;
    float reflector;
    vec3 indirectLight;
    float lightForce;
    vec3 origin;
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
struct MaterialData {
	float albedoIntensity;
	float normalIntensity;
	float specularIntensity;
	float roughnessIntensity;
    vec3 diffuseColor;
    int reflector;
	int texturesIndexStart; 
	int textureSizes; 
	int diffuseOffset;
    int normalOffset;
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

layout(set = 0,binding = 10) uniform sampler2D textures[];

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

#define MAX_TEXTURES 5

vec4 CurrentMaterialTextures[MAX_TEXTURES];
int texturesOnMaterialCount = 0;

vec3 GetDiffuseColor(int materialIndex);
void FillTexturesFromMaterial(int texturesIndexStart, int textureSizes, vec2 uv);
vec4 TryGetTex(int texIndexStart, int texOffset, vec2 uv);
vec4 GetColorOrDiffuseTex(vec2 uv);

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
  normal=normalize(normal);



  //materials

  int materialIndex= meshesData[realGeometryOffset].materialIndexOnShape;
  int materialIndexInTextures=materials[materialIndex].texturesIndexStart;
  int materialTextureSizes =materials[materialIndex].textureSizes;
  
  vec4 diffuseInMat = TryGetTex(materialIndexInTextures, materials[materialIndex].diffuseOffset, uv);
  vec4 normalInMat = TryGetTex(materialIndexInTextures, materials[materialIndex].normalOffset, uv);
  MaterialFindInfo matInfo = GetMatInfo(diffuseInMat, normalInMat);
  
  
  //FillTexturesFromMaterial(materialIndexInTextures, materialTextureSizes, uv); 
  
  vec4 diffuse=diffuseInMat;

  vec3 spec = vec3(0.8f); 
  vec3 view = normalize(pos-rayPayload.origin);
  vec3 lightDir= normalize(pos-myLight.pos); 
  vec3 halfway =normalize(view + lightDir);
  vec3 pbr= GetPBR(diffuse.xyz, myLight.col, 0.0f, 0.0f, 0.0f, spec, normal, view, lightDir, halfway);
 
  float shadingIntensity= GetLightShadingIntensity(pos, myLight.pos, normal);
  
  //rayPayload.color = (diffuse.xyz * myLight.col * rayPayload.indirectLight) * shadingIntensity * myLight.intensity * rayPayload.lightForce; 
  rayPayload.color = pbr * myLight.intensity; 
  rayPayload.distance = gl_RayTmaxEXT;
  rayPayload.normal = normal;
  rayPayload.indirectLight = diffuse.xyz;
  rayPayload.lightForce -= 0.3f;
  if(materials[materialIndex].reflector == 1){
    rayPayload.reflector = 1.0f;
  }else{
    rayPayload.reflector = 0.0f;
  }

  //vec3 debuging=GetDebugCol(primitiveIndex,  575262.0);
  //vec3 debugGeometryIndex=GetDebugCol(materialIndex,  4);
  //rayPayload.color = debuging;
  //rayPayload.color = debugGeometryIndex;
  //rayPayload.color = normal * 0.5 + 0.5; // Uncomment for normal debugging
  //rayPayload.color = normDebug;
}

vec4 TryGetTex(int texIndexStart, int texOffset, vec2 uv){
    if (texOffset== -1){
        return vec4(0, 1, 1, 1);
    }
    vec4 texture = texture(textures[texIndexStart + texOffset],uv);
    return texture;
}
void FillTexturesFromMaterial(int texturesIndexStart, int textureSizes, vec2 uv){

    int textureFinishSize = texturesIndexStart + textureSizes;
    int allTexturesIndex= 0;
    texturesOnMaterialCount= allTexturesIndex;
    for(int i = texturesIndexStart; i < textureFinishSize; i++){

        if(allTexturesIndex>MAX_TEXTURES){
            return;
        }
        CurrentMaterialTextures[allTexturesIndex] = texture(textures[i],uv); 
        allTexturesIndex++;
        texturesOnMaterialCount= allTexturesIndex;
    }

}

vec3 GetDiffuseColor(int materialIndex){

   vec3 diffuse= materials[materialIndex].diffuseColor;
   return diffuse;
}
vec4 GetColorOrDiffuseTex(vec2 uv){

    if(texturesOnMaterialCount>0){
        vec4 diffuseText = CurrentMaterialTextures[DIFFUSE_TEX];
        return diffuseText;
    }else{
        int index= meshesData[gl_GeometryIndexEXT].materialIndexOnShape;
        vec3 diffuseCol=GetDiffuseColor(index); 
        return vec4(diffuseCol, 1.0);
    }


}


