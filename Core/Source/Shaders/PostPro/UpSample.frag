#version 450 core

//layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0, rgba8) uniform image2D srcTexture;
layout(set = 0, binding = 1, rgba8) uniform image2D upSample;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;


void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float filterRadius = 1.0f;
    float x = filterRadius;
    float y = filterRadius;
    ivec2 fragCoord = ivec2(gl_FragCoord.xy);
    vec4 upSampleColor = vec4(0.0f);
    
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec4 a = imageLoad(srcTexture, ivec2(fragCoord.x - x, fragCoord.y + y));
    vec4 b = imageLoad(srcTexture, ivec2(fragCoord.x,     fragCoord.y + y));
    vec4 c = imageLoad(srcTexture, ivec2(fragCoord.x + x, fragCoord.y + y));

    vec4 d = imageLoad(srcTexture, ivec2(fragCoord.x - x, fragCoord.y));
    vec4 e = imageLoad(srcTexture, ivec2(fragCoord.x,     fragCoord.y));
    vec4 f = imageLoad(srcTexture, ivec2(fragCoord.x + x, fragCoord.y));

    vec4 g = imageLoad(srcTexture, ivec2(fragCoord.x - x, fragCoord.y - y));
    vec4 h = imageLoad(srcTexture, ivec2(fragCoord.x,     fragCoord.y - y));
    vec4 i = imageLoad(srcTexture, ivec2(fragCoord.x + x, fragCoord.y - y));

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    upSampleColor = e*4.0;
    upSampleColor += (b+d+f+h)*2.0;
    upSampleColor += (a+c+g+i);
    upSampleColor *= 1.0 / 16.0;
    imageStore(upSample, fragCoord, upSampleColor);

}