#version 450

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 myColors;

layout (binding = 0) uniform UniformBufferObjectData{
	mat4 model; 
	mat4 view; 
	mat4 projection; 
} ubo;
 

void main(){
	gl_Position = ubo.projection * ubo.view * ubo.model *  vec4(position, 0.0, 1.0);
	myColors= color;

}