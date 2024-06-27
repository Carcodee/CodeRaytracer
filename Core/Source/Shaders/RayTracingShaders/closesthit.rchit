#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require



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
    float padding1;
    vec3 diffuseColor;
    float padding2;
	int texturesIndexStart; 
	int textureSizes; 
	int meshIndex; 
    int padding3;
};

struct MeshData {
    int materialIndexOnShape;
    int geometryIndexStartOffset;
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


layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

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

layout(set = 0,binding = 9) uniform sampler2D textures[];

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
#define DIFFUSE_TEX 0 
#define ALPHA_TEX 1 
#define SPECULAR_TEX 2 
#define BUMP_TEX 3 
#define AMBIENT_TEX 4 

vec4 CurrentMaterialTextures[MAX_TEXTURES];
int texturesOnMaterialCount = 0;

vec3 GetDiffuseColor(int materialIndex);
void FillTexturesFromMaterial(int texturesIndexStart, int textureSizes, vec2 uv);
vec4 GetColorOrDiffuseTex(vec2 uv);

void main()
{
    

  int primitiveIndex=int(meshesData[gl_GeometryIndexEXT].geometryIndexStartOffset);

  int idx1= primitiveIndex + (3 * gl_PrimitiveID + 0);
  int idx2= primitiveIndex + (3 * gl_PrimitiveID + 1);
  int idx3= primitiveIndex + (3 * gl_PrimitiveID + 2);
  int index1= int(indices[idx1].index);
  int index2= int(indices[idx2].index);
  int index3= int(indices[idx3].index);

  Vertex v1 = vertices[index1];
  Vertex v2 = vertices[index2];
  Vertex v3 = vertices[index3];
  
  const vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  vec2 uv = barycentricCoords.x * v1.texCoords + barycentricCoords.y * v2.texCoords + barycentricCoords.z * v3.texCoords;
  vec3 pos= barycentricCoords.x * v1.position + barycentricCoords.y * v2.position + barycentricCoords.z * v3.position;
  vec3 normal= barycentricCoords.x * v1.normal + barycentricCoords.y * v2.normal + barycentricCoords.z * v3.normal;
  normal=normalize(normal);

  //materials

  int materialIndex= meshesData[gl_GeometryIndexEXT].materialIndexOnShape;
  int materialIndexInTextures=materials[materialIndex].texturesIndexStart;
  int materialTextureSizes =materials[materialIndex].textureSizes;

  FillTexturesFromMaterial(materialIndexInTextures, materialTextureSizes, uv); 
  
  vec4 diffuse=GetColorOrDiffuseTex(uv);

  
  float shadingIntensity= GetLightShadingIntensity(pos, myLight.pos, normal);
  hitValue = (diffuse.xyz * myLight.col) * shadingIntensity * myLight.intensity;
  //hitValue = diffuse.xyz;
  //1724928
  //550091
//
//  if(gl_GeometryIndexEXT == 21){
//  
//    hitValue = vec3(1.0, 0.0, 0.0);
//    if(primitiveIndex>=1724928){
//        hitValue = vec3(0.0, 0.0, 1.0);
//    }
//  }
//  else{
//    hitValue = vec3(0.0); 
//  }

  //vec3 debuging=GetDebugCol(primitiveIndex,  575262.0);
  //vec3 debugGeometryIndex=GetDebugCol(materialIndex,  4);
  //hitValue = debuging;
  //hitValue = debugGeometryIndex;
  //hitValue = normal * 0.5 + 0.5; // Uncomment for normal debugging
  //hitValue = normDebug;
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

