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
    ivec2 sizeMip5 = imageSize(mip_5_EmissiveImage);
    ivec2 sizeMip0 = imageSize(mip_0_EmissiveImage);

    float ratioX = float(sizeMip5.x) / float(sizeMip0.x);
    float ratioY = float(sizeMip5.y) / float(sizeMip0.y);

    ivec2 coord = ivec2(gl_FragCoord.x * ratioX, gl_FragCoord.y * ratioY);

    // Ensure coordinates are within bounds

    vec4 color = vec4(0.0);
    int blurRadius = 4;
    int kernelSize = (blurRadius * 2 + 1) * (blurRadius * 2 + 1);

    for (int x = -blurRadius; x <= blurRadius; x++) {
        for (int y = -blurRadius; y <= blurRadius; y++) {
            ivec2 sampleCoord = coord + ivec2(x, y);
            color += imageLoad(mip_5_EmissiveImage, sampleCoord);
            // Ensure sampling coordinates are within bounds
//            if (sampleCoord.x >= 0 && sampleCoord.x < sizeMip5.x && sampleCoord.y >= 0 && sampleCoord.y < sizeMip5.y) {
//            }
        }
    }

    color /= float(kernelSize);  // Normalize the color by the kernel size

    // Ensure the coordinates for imageStore are within bounds
    ivec2 realCoords = ivec2(gl_FragCoord.xy);
//    if (realCoords.x >= 0 && realCoords.x < sizeMip0.x && realCoords.y >= 0 && realCoords.y < sizeMip0.y) {
        imageStore(mip_0_EmissiveImage, coord, color);
//    }
}
