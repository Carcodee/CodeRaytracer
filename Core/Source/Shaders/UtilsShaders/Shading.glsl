
#include "../UtilsShaders/Functions.glsl"

#ifndef SHADING 
#define SHADING 

float AbsCosTheta(vec3 x, vec3 y){
    return max(dot(x,y),0.001f);
}

float CosTheta(vec3 x, vec3 y){
    return dot(x,y);
}

vec3 Schlick(vec3 r0, float radians)
{
    float exponential = pow(1.0f - radians, 5.0f);
    return r0 + (vec3(1.0f) - r0) * exponential;
}

float SchlickWeight(float u)
{
    float m = Saturate(1.0f - u);
    float m2 = m * m;
    return m * m2 * m2;
}

float SchlickR0FromRelativeIOR(float eta)
{
    return pow(eta - 1.0f, 2.0f) / pow(eta + 1.0f, 2.0f);
}

float Schlick(float r0, float radians)
{
    return Lerp(1.0f, SchlickWeight(radians), r0);
}
float SchlickDielectic(float cosThetaI, float relativeIor)
{
    float r0 = SchlickR0FromRelativeIOR(relativeIor);
    return r0 + (1.0f - r0) * SchlickWeight(cosThetaI);
}

float Dielectric(float cosThetaI, float ni, float nt)
{
    // Copied from PBRT. This function calculates the full Fresnel term for a dielectric material.
    // See Sebastion Legarde's link above for details.

    cosThetaI = clamp(cosThetaI, -1.0f, 1.0f);

    // Swap index of refraction if this is coming from inside the surface
    if(cosThetaI < 0.0f) {
        float temp = ni;
        ni = nt;
        nt = temp;

        cosThetaI = -cosThetaI;
    }

    float sinThetaI = sqrt(max(1.0f - cosThetaI * cosThetaI, 0.0f));
    float sinThetaT = ni / nt * sinThetaI;

    // Check for total internal reflection
    if(sinThetaT >= 1) {
        return 1;
    }

    float cosThetaT = sqrt(max(1.0f - sinThetaT * sinThetaT,0.0f));

    float rParallel     = ((nt * cosThetaI) - (ni * cosThetaT)) / ((nt * cosThetaI) + (ni * cosThetaT));
    float rPerpendicuar = ((ni * cosThetaI) - (nt * cosThetaT)) / ((ni * cosThetaI) + (nt * cosThetaT));
    return (rParallel * rParallel + rPerpendicuar * rPerpendicuar) / 2;
}

#endif 