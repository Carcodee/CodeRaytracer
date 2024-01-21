#version 450

layout(location = 0) out vec4 outColor;

layout (location = 0) in vec3 myColors;

void main() {

	outColor= vec4(myColors, 1.0);

}