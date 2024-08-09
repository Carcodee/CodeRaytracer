#version 450 core

//layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0, rgba8) uniform image2D upSample;
layout(set = 0, binding = 1, rgba8) uniform image2D downSampledTex;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

layout(push_constant) uniform pushConstantVariables{
    float bloomSize;
} pc;

void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float filterRadius = pc.bloomSize;
    ivec2 aspectRatio =imageSize(upSample)/imageSize(downSampledTex);
    float x = filterRadius / aspectRatio.x;
    float y = filterRadius / aspectRatio.y;
    ivec2 fragCoordDownSampled = ivec2(gl_FragCoord.xy) / aspectRatio;
    vec4 upSampleColor = vec4(0.0f);
    
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec4 a = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x - x, fragCoordDownSampled.y + y));
    vec4 b = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x,     fragCoordDownSampled.y + y));
    vec4 c = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x + x, fragCoordDownSampled.y + y));

    vec4 d = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x - x, fragCoordDownSampled.y));
    vec4 e = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x,     fragCoordDownSampled.y));
    vec4 f = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x + x, fragCoordDownSampled.y));

    vec4 g = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x - x, fragCoordDownSampled.y - y));
    vec4 h = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x,     fragCoordDownSampled.y - y));
    vec4 i = imageLoad(downSampledTex, ivec2(fragCoordDownSampled.x + x, fragCoordDownSampled.y - y));

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    upSampleColor = e*4.0;
    upSampleColor += (b+d+f+h)*2.0;
    upSampleColor += (a+c+g+i);
    upSampleColor *= 1.0 / 16.0;

    ivec2 fragCoord = ivec2(gl_FragCoord.xy);
    imageStore(upSample, fragCoord , upSampleColor);

}