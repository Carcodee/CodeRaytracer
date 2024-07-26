#version 450 core

//layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0, rgba8) uniform image2D mip_5_EmissiveImage;
layout(set = 0, binding = 1, rgba8) uniform image2D mip_0_EmissiveImage;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;


void main()
{

}