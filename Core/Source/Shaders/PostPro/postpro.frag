#version 450 core

//layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0, rgba8) uniform image2D image;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;


void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec4 color = vec4(0.0);
    int blurRadius = 4;  
    int kernelSize = (blurRadius * 2 + 1) * (blurRadius * 2 + 1); 
    for (int x = -blurRadius; x <= blurRadius; x++) {
        for (int y = -blurRadius; y <= blurRadius; y++) {
            color += imageLoad(image, coord + ivec2(x, y));
        }
    }
    
    color /= float(kernelSize);  // Normalize the color by the kernel size
    imageStore(image, coord,color);
}
