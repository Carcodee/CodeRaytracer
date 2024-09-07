
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

//===================================================================================================================
float SeparableSmithGGXG1(vec3 w, float a)
{
    float a2 = a * a;
    float absDotNV = AbsCosTheta(w);

    return 2.0f / (1.0f + sqrt(a2 + (1 - a2) * absDotNV * absDotNV));
}

float SeparableSmithGGXG1(vec3 w, vec3 wl, float ax, float ay)
{
    float absTanTheta = abs(TanTheta(w));
    if(isinf(absTanTheta)) {
        return 0.0f;
    }
    float a = sqrt(Cos2Phi(w) * ax * ax + Sin2Phi(w) * ay * ay);
    float a2Tan2Theta = pow(a * absTanTheta, 2.0f);

    float lambda = 0.5f * (-1.0f + sqrt(1.0f + a2Tan2Theta));
    return 1.0f / (1.0f + lambda);
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
//=========================================================================================================================
float GgxAnisotropicD(vec3 wm, float ax, float ay)
{
    float dotHX2 = (wm.x, 2.0f);
    float dotHY2 = (wm.z, 2.0f);
    float cos2Theta = pow(CosTheta(wm),2.0f);
    float ax2 = (ax, 2.0f);
    float ay2 = (ay, 2.0f);

    return 1.0f / (PI * ax * ay * pow(dotHX2 / ax2 + dotHY2 / ay2 + cos2Theta, 2.0f));
}

//=========================================================================================================================
vec3 SampleGgxVndfAnisotropic(vec3 wo, float ax, float ay, float u1, float u2)
{
    // -- Stretch the view vector so we are sampling as though roughness==1
    vec3 v = normalize(vec3(wo.x * ax, wo.y, wo.z * ay));

    // -- Build an orthonormal basis with v, t1, and t2
    vec3 t1 = (v.y < 0.9999f) ? normalize(cross(v, vec3(0, 1.0f, 0.0f))) : vec3(1.0f, 0.0f, 0.0f);
    vec3 t2 = cross(t1, v);

    // -- Choose a point on a disk with each half of the disk weighted proportionally to its projection onto direction v
    float a = 1.0f / (1.0f + v.y);
    float r = sqrt(u1);
    float phi = (u2 < a) ? (u2 / a) * PI : PI + (u2 - a) / (1.0f - a) * PI;
    float p1 = r * cos(phi);
    float p2 = r * sin(phi) * ((u2 < a) ? 1.0f : v.y);

    // -- Calculate the normal in this stretched tangent space
    vec3 n = p1 * t1 + p2 * t2 + sqrt(max(1.0f - p1 * p1 - p2 * p2, 0.0f)) * v;

    // -- unstretch and normalize the normal
    return normalize(vec3(ax * n.x, n.y, ay * n.z));
}
void GgxVndfAnisotropicPdf(vec3 wi, vec3 wm, vec3 wo, float ax, float ay, inout float forwardPdfW, inout float reversePdfW)
{
    float D = GgxAnisotropicD(wm, ax, ay);

    float absDotNL = AbsCosTheta(wi);
    float absDotHL = abs(dot(wm, wi));
    float G1v = SeparableSmithGGXG1(wo, wm, ax, ay);
    forwardPdfW = G1v * absDotHL * D / absDotNL;

    float absDotNV = AbsCosTheta(wo);
    float absDotHV = abs(dot(wm, wo));
    float G1l = SeparableSmithGGXG1(wi, wm, ax, ay);
    reversePdfW = G1l * absDotHV * D / absDotNV;
}

bool Transmit(vec3 wm, vec3 wi, float n, vec3 wo)
{
    float c = dot(wi, wm);
    if(c < 0.0f) {
        c = -c;
        wm = -wm;
    }

    float root = 1.0f - n * n * (1.0f - c * c);
    if(root <= 0)
    return false;

    wo = (n * c - sqrt(root)) * wm - n * wi;
    return true;
}
float ThinTransmissionRoughness(float ior, float roughness)
{
    // -- Disney scales by (.65 * eta - .35) based on figure 15 of the 2015 PBR course notes. Based on their figure the results
    // -- match a geometrically thin solid fairly well but it is odd to me that roughness is decreased until an IOR of just
    // -- over 2.
    return Saturate((0.65f * ior - 0.35f) * roughness);
}

#endif 