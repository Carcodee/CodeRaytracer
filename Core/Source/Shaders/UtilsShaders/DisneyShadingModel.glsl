
#include "../UtilsShaders/Random.glsl"
#include "../UtilsShaders/Functions.glsl"
#include "../UtilsShaders/Shading.glsl"

vec2 GetAnisotropic(float roughness, float anisotropic) {
    float aspect = sqrt(1.0 - 0.9 * anisotropic);
    float roughness_sqr = roughness * roughness;
    return vec2(max(0.001, roughness_sqr / aspect), max(0.001, roughness_sqr * aspect));
}

//=============================================================================================================================
void CalculateLobePdfs(MaterialData material,float pSpecular, float pDiffuse, float pClearcoat, float pSpecTrans)
{
    float metallicBRDF   = material.metallicIntensity;
    float specularBSDF   = (1.0f - material.metallicIntensity) * material.specularTransmission;
    float dielectricBRDF = (1.0f - material.specularTransmission) * (1.0f - material.metallicIntensity);

    float specularWeight     = metallicBRDF + dielectricBRDF;
    float transmissionWeight = specularBSDF;
    float diffuseWeight      = dielectricBRDF;
    float clearcoatWeight    = 1.0f * Saturate(material.clearcoat);

    float norm = 1.0f / (specularWeight + transmissionWeight + diffuseWeight + clearcoatWeight);

    pSpecular  = specularWeight     * norm;
    pSpecTrans = transmissionWeight * norm;
    pDiffuse   = diffuseWeight      * norm;
    pClearcoat = clearcoatWeight    * norm;
}


//Sheen
//=============================================================================================================================
vec3 CalculateTint(vec3 baseColor)
{
    float luminance = dot(vec3(0.3f, 0.6f, 1.0f), baseColor);
    return (luminance > 0.0f) ? baseColor * (1.0f / luminance) : vec3(1.0f);
}

//=============================================================================================================================
vec3 EvaluateSheen(MaterialData material, vec3 wo, vec3 wl, vec3 wi)
{
    if(material.sheen <= 0.0f) {
        return vec3(0.0f);
    }
    float dotHL = dot(wl, wi);
    vec3 tint = CalculateTint(material.diffuseColor.xyz);
    return material.sheen * Lerp(vec3(1.0f), tint, material.sheenTint) * SchlickWeight(dotHL);
}

//Clearcoat
//===================================================================================================================
float GTR1(float absDotHL, float a)
{
    if(a >= 1) {
        return INV_PI;
    }

    float a2 = a * a;
    return (a2 - 1.0f) / (PI * log2(a2) * (1.0f + (a2 - 1.0f) * absDotHL * absDotHL));
}

//===================================================================================================================
float SeparableSmithGGXG1(vec3 w,vec3 n, float a)
{
    float a2 = a * a;
    float absDotNV = AbsCosTheta(w, n);

    return 2.0f / (1.0f + sqrt(a2 + (1 - a2) * absDotNV * absDotNV));
}