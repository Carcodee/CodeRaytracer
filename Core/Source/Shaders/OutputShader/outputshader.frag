#version 450 core

//layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0, rgba8) uniform image2D finalImage;
layout(set = 0, binding = 1, rgba8) uniform image2D emmisiveImage;
layout(set = 0, binding = 2, rgba8) uniform image2D raytracingImage;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec4 emissiveCol= imageLoad(emmisiveImage, coord);
    vec4 rayTracingCol= imageLoad(raytracingImage, coord);
    
    vec4 finalCol = emissiveCol + rayTracingCol;

    imageStore(finalImage, coord, finalCol);
    
}