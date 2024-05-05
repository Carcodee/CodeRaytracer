#version 450

layout(location = 0) out vec4 outColor;

layout (location = 0) in vec3 myColors;
layout (location = 1) in vec2 textCoord;


layout(set=0 , binding = 1) uniform sampler2D texSampler;
layout(set=0 , binding = 2) uniform sampler2D particles;
layout(set=0 , binding = 3) uniform sampler2D raytracingImage;

void main() {


	vec4 particles= texture(particles, textCoord);
	vec4 modelTex= texture(texSampler, textCoord);
	vec4 raytracingTex= texture(raytracingImage, textCoord);
	//outColor = particles + modelTex;

//	outColor = raytracingTex+ modelTex;
	//outColor= texture(texSampler, textCoord);

	outColor = vec4(textCoord,0.0,1.0);
}