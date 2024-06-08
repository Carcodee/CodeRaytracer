#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require


layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

layout(set=0, binding=3) uniform sampler2D mySampler;



struct MaterialData {
	float albedoIntensity;
	float normalIntensity;
	float specularIntensity;
	int textureIndex; 
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

layout(set = 0, binding = 4, scalar) buffer VertexBuffer {
    Vertex vertices[];
};

layout(set = 0, binding = 5, scalar) buffer IndexBuffer {
    Indices indices[];
};

layout(binding=6) uniform light{
    vec3 pos;
    vec3 col;
    float intensity;
}myLight;

layout(binding = 7, set = 0) uniform sampler2D textures[];

vec3 GetDebugCol(uint primitiveId, float primitiveCount){

    float idNormalized = float(primitiveId) / primitiveCount; 
    vec3 debugColor = vec3(idNormalized, 1.0 - idNormalized, 0.5 * idNormalized);
    return debugColor;
}
float GetLightShadingIntensity(vec3 fragPos, vec3 lightPos, vec3 normal){
    
    vec3 dir= fragPos-lightPos; 
    dir= normalize(dir);
    float colorShading=max(dot(dir, normal),0.0);
    return colorShading;

}

void main()
{

  int index1= int(indices[3 * gl_PrimitiveID + 0].index);
  int index2= int(indices[3 * gl_PrimitiveID + 1].index);
  int index3= int(indices[3 * gl_PrimitiveID + 2].index);

  Vertex v1 = vertices[index1];
  Vertex v2 = vertices[index2];
  Vertex v3 = vertices[index3];

  const vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  vec2 uv = barycentricCoords.x * v1.texCoords + barycentricCoords.y * v2.texCoords + barycentricCoords.z * v3.texCoords;
  vec3 pos= barycentricCoords.x * v1.position + barycentricCoords.y * v2.position + barycentricCoords.z * v3.position;
  vec3 normal= normalize(barycentricCoords.x * v1.normal + barycentricCoords.y * v2.normal + barycentricCoords.z * v3.normal);
  
  float shadingIntensity= GetLightShadingIntensity(pos, myLight.pos, normal);

  vec4 textSample=texture(textures[0],uv);
  vec3 debuging=GetDebugCol(gl_PrimitiveID,  3828.0);


  vec3 debugGeometryIndex=GetDebugCol(gl_GeometryIndexEXT,  6);
  


  hitValue = (textSample.xyz * myLight.col) * shadingIntensity * myLight.intensity;
  //hitValue = debugGeometryIndex;
  }