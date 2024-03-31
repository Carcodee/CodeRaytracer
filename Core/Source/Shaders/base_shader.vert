#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTextCoord;

layout (location = 0) out vec3 myColors;
layout (location = 1) out vec2 textCoord;

layout (binding = 0) uniform UniformBufferObjectData{
	mat4 model; 
	mat4 view; 
	mat4 projection; 
} ubo;
 

void main(){
	gl_Position = ubo.projection * ubo.view * ubo.model *  vec4(inPosition, 1.0);
	myColors= inColor;
	textCoord= inTextCoord;
}