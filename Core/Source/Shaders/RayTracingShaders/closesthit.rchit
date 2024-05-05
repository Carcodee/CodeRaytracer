#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

layout(set=0, binding=3) uniform sampler2D mySampler;

struct Vertex {
    vec3 position; 
    vec3 col;
    vec2 textCoords; 
};

struct Indices {
    uint index;
};

layout(std140, binding = 4 ) buffer VertexBuffer {
    Vertex vertices[];
};

layout(std140, binding = 5 ) buffer IndexBuffer {
    Indices indices[];
};

vec3 GetDebugCol(uint primitiveId, float primitiveCount){

    float idNormalized = float(primitiveId) / primitiveCount; 
    vec3 debugColor = vec3(idNormalized, 1.0 - idNormalized, 0.5 * idNormalized);
    return debugColor;
}

void main()
{

  int index1= indices[3 * gl_PrimitiveID + 0].index;
  int index2= indices[3 * gl_PrimitiveID + 1].index;
  int index3= indices[3 * gl_PrimitiveID + 2].index;

  Vertex v1 = vertices[index1];
  Vertex v2 = vertices[index2];
  Vertex v3 = vertices[index3];

  const vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  vec2 uv = barycentricCoords.x*v1.textCoords + barycentricCoords.y*v2.textCoords + barycentricCoords.z*v3.textCoords;

  vec4 textSample=texture(mySampler,uv);
  //vec3 debuging=GetDebugCol(gl_PrimitiveID,  3828.0);
  hitValue =textSample.xyz;
}