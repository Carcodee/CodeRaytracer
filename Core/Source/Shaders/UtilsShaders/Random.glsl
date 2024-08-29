

#ifndef RANDOM 
#define RANDOM 

#define PI 3.1415
#define INV_PI 0.3183

float CosinePdfHemisphere(float cosTheta)
{
    return cosTheta / PI;
}

vec3 CosineSampleHemisphere(vec2 E)
{
    const float phi = 2.0 * PI * E.x;
    const float cosTheta = sqrt(E.y);
    const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;
    return H;
}


uint GetHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed = seed + (seed << 3);
    seed = seed ^ (seed >> 4);
    seed = seed * 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

uint rotl(uint x, uint k)
{
    return (x << k) | (x >> (32 - k));
}
uint Rand(inout uvec2 seed)
{
    const uint result = rotl(seed.x * 0x9E3779BB, 5) * 5;
    seed.y ^= seed.x;
    seed.x = rotl(seed.x, 26) ^ seed.y ^ (seed.y << 9);
    seed.y = rotl(seed.y, 13);

    return result;
}
uvec2 GetSeed(uvec2 id, uint sampleIndex, uint accumulationIndex)
{
    const uint s0 = (id.x << 16) | id.y;
    const uint s1 = (sampleIndex << 8) | accumulationIndex;

    uvec2 seed = uvec2(GetHash(s0), GetHash(s1));
    Rand(seed);

    return seed;
}
float NextFloat(inout uvec2 seed)
{
    uint u = 0x3F800000 | (Rand(seed) >> 9);
    return uintBitsToFloat(u) - 1.0;
}

vec2 NextVec2(inout uvec2 seed)
{
    return vec2(NextFloat(seed), NextFloat(seed));
}

vec3 NextVec3(inout uvec2 seed)
{
    return vec3(NextFloat(seed), NextFloat(seed), NextFloat(seed));
}

#endif 
