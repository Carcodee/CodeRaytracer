#version 450 core

//layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0, rgba8) uniform image2D srcTexture;
layout(set = 0, binding = 1, rgba8) uniform image2D downSample;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

void main()
{
    vec4 downSampleColor = vec4(0.0f);
    ivec2 aspectRatio =imageSize(srcTexture)/imageSize(downSample);
    vec2 srcTexelSize = vec2(1.0f);
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    ivec2 fragCoord = ivec2(gl_FragCoord.xy);
    
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec4 a = imageLoad(srcTexture, ivec2(fragCoord.x - 2 * x, fragCoord.y + 2 * y));
    vec4 b = imageLoad(srcTexture, ivec2(fragCoord.x,         fragCoord.y + 2 * y));
    vec4 c = imageLoad(srcTexture, ivec2(fragCoord.x + 2 * x, fragCoord.y + 2 * y));

    vec4 d = imageLoad(srcTexture, ivec2(fragCoord.x - 2 * x, fragCoord.y));  
    vec4 e = imageLoad(srcTexture, ivec2(fragCoord.x,         fragCoord.y));  
    vec4 f = imageLoad(srcTexture, ivec2(fragCoord.x + 2 * x, fragCoord.y));  

    vec4 g = imageLoad(srcTexture, ivec2(fragCoord.x - 2 * x, fragCoord.y - 2 * y));
    vec4 h = imageLoad(srcTexture, ivec2(fragCoord.x,         fragCoord.y - 2 * y));
    vec4 i = imageLoad(srcTexture, ivec2(fragCoord.x + 2 * x, fragCoord.y - 2 * y));

    vec4 j = imageLoad(srcTexture, ivec2(fragCoord.x - x ,  fragCoord.y + y));
    vec4 k = imageLoad(srcTexture, ivec2(fragCoord.x + x ,  fragCoord.y + y));
    vec4 l = imageLoad(srcTexture, ivec2(fragCoord.x - x ,  fragCoord.y - y));
    vec4 m = imageLoad(srcTexture, ivec2(fragCoord.x + x ,  fragCoord.y - y));

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    downSampleColor = e*0.125;
    downSampleColor += (a+c+g+i)*0.03125;
    downSampleColor += (b+d+f+h)*0.0625;
    downSampleColor += (j+k+l+m)*0.125;
//    imageStore(downSample, fragCoord, vec4(fragCoord / vec2(600.0, 450.0), 0.0f, 1.0f));
    imageStore(downSample, fragCoord/aspectRatio, downSampleColor);

}