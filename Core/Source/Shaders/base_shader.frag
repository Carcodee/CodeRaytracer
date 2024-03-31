#version 450

layout(location = 0) out vec4 outColor;

layout (location = 0) in vec3 myColors;
layout (location = 1) in vec2 textCoord;


layout(set=0 , binding = 1) uniform sampler2D texSampler;
layout(set=0 , binding = 2) uniform sampler2D particles;

void main() {


	vec4 particles= texture(particles, textCoord);
	vec4 modelTex= texture(texSampler, textCoord);
	outColor = particles + modelTex;

	//outColor= texture(texSampler, textCoord);

}