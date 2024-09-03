
#include "../UtilsShaders/Functions.glsl"
#include "../UtilsShaders/Random.glsl"

#ifndef SHADING 
#define SHADING 

float AbsCosThetaWS(vec3 x, vec3 y){
    return max(dot(x,y),0.001f);
}
float AbsCosTheta(vec3 x){
    return max(x.z,.001f);
}


float CosTheta(vec3 x, vec3 y){
    return dot(x,y);
}

float CosTheta(vec3 x){
    return x.z;
}
float Sin2Phi(vec3 w) {
    float sinPhi = w.y / sqrt(w.x * w.x + w.y * w.y);
    return sinPhi * sinPhi;
}

float Cos2Phi(vec3 w) {
    float cosPhi = w.x / sqrt(w.x * w.x + w.y * w.y);
    return cosPhi * cosPhi;
}
float TanTheta(vec3 w) {
    return sqrt(1.0 - w.z * w.z) / w.z;
}

vec3 Schlick(vec3 r0, float radians)
{
    float exponential = pow(1.0f - radians, 5.0f);
    return r0 + (vec3(1.0f) - r0) * exponential;
}

//=============================================================================================================================
vec3 CalculateExtinction(vec3 apparantColor, float scatterDistance)
{
    vec3 a = apparantColor;
    vec3 a2 = a * a;
    vec3 a3 = a2 * a;

    vec3 alpha =    vec3(1.0f) - exp(-5.09406f * a + 2.61188f * a2 - 4.31805f * a3);
    vec3 s = vec3(1.9f) - a + 3.5f * (a - vec3(0.8f)) * (a - vec3(0.8f));
    return 1.0f / (s * scatterDistance);
}
float SmithGGX(float NdotX, float roughness) {
    float a = roughness * roughness;
    float k = (a * 0.5);
    float denom = NdotX * (1.0 - k) + k;
    return NdotX / denom;
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
float SchlickFresnel(float cosTheta) {
    float F0 = 0.04; // or some constant value
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
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

float GTR2(float NdotH, float alpha) {
    float alpha2 = alpha * alpha;
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (PI * denom * denom);
}
vec3 ImportanceSampleGTR2(float roughness, float r1, float r2) {
    float a = roughness * roughness; // Square the roughness to get the distribution parameter

    // Convert r1 and r2 (random values) to spherical coordinates
    float phi = 2.0 * PI * r1; // r1 controls the azimuthal angle
    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a * a - 1.0) * r2)); // r2 controls the elevation angle
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // Spherical to Cartesian conversion
    vec3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    // Normalize H to ensure it's a unit vector
    return normalize(H);
}
vec3 ImportanceSampleGTR1(float alpha, float r1, float r2) {
    float phi = 2.0 * PI * r1; // Sample the azimuthal angle uniformly
    float cosTheta = sqrt((1.0 - pow(alpha, 2.0)) / (1.0 + (pow(alpha, 2.0) - 1.0) * r2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    return normalize(H);
}
#endif 