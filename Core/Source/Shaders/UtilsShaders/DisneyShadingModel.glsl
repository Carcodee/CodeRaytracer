
#include "../UtilsShaders/Random.glsl"
#include "../UtilsShaders/Functions.glsl"
#include "../UtilsShaders/Shading.glsl"

#ifndef DISNEYBSDF 
#define DISNEYBSDF 

vec2 GetAnisotropic(float roughness, float anisotropic) {
    float aspect = sqrt(1.0 - 0.9 * anisotropic);
    float roughness_sqr = roughness * roughness;
    return vec2(max(roughness_sqr / aspect, 0.001), max( roughness_sqr * aspect, 0.001));
}

vec3 CalculateTint(vec3 baseColor)
{
    float luminance = dot(vec3(0.3f, 0.6f, 1.0f), baseColor);
    return (luminance > 0.0f) ? baseColor * (1.0f / luminance) : vec3(1.0f);
}

vec3 DisneyFresnel(MaterialData material, vec3 wo, vec3 wm, vec3 wi)
{
    float dotHV = dot(wm, wo);

    vec3 tint = CalculateTint(material.diffuseColor.xyz);

    // -- See section 3.1 and 3.2 of the 2015 PBR presentation + the Disney BRDF explorer (which does their
    // -- 2012 remapping rather than the SchlickR0FromRelativeIOR seen here but they mentioned the switch in 3.2).
    //relative ior is goignt to be 1.0f so it will be the ior (ior/ior surrounding medium)
    vec3 R0 = SchlickR0FromRelativeIOR(material.relativeIOR) * Lerp(vec3(1.0f), tint, material.specularTint);
    R0 = Lerp(R0, material.diffuseColor.xyz, material.metallicIntensity);

    float dielectricFresnel = Dielectric(dotHV, 1.0f, material.ior);
    vec3 metallicFresnel = Schlick(R0, dot(wi, wm));

    return Lerp(vec3(dielectricFresnel), metallicFresnel, material.metallicIntensity);

}

//=============================================================================================================================
void CalculateLobePdfs(MaterialData material,inout float pSpecular,inout  float pDiffuse,inout  float pClearcoat,inout  float pSpecTrans)
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

//=============================================================================================================================
vec3 EvaluateSheen(MaterialData material, vec3 wo, vec3 wm, vec3 wi)
{
    if(material.sheen <= 0.0f) {
        return vec3(0.0f);
    }
    float dotHL = dot(wm, wi);
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
float EvaluateDisneyClearcoat(float clearcoat, float alpha, vec3 wo, vec3 wm, vec3 wi, inout float fPdfW, inout float rPdfW){
    if(clearcoat <= 0.0f){
        return 0.0f;
    }

    float absDotNH = AbsCosTheta(wm);
    float absDotNL = AbsCosTheta(wi);
    float absDotNV = AbsCosTheta(wo);
    float dotHL = dot(wm, wi);
    float d = GTR1(absDotNH, Lerp(0.1f, 0.001f, alpha));
    float f = Schlick(0.04f, dotHL);
    float gl = SeparableSmithGGXG1(wi, 0.25f);
    float gv = SeparableSmithGGXG1(wo, 0.25f);

    fPdfW = d / (4.0f * absDotNL);
    rPdfW = d / (4.0f * absDotNV);

    return 0.25f * clearcoat * d * f * gl * gv;
}

//spec brdf
//===================================================================================================================


//===================================================================================================================
float AnisotropicGgxDistribution(vec3 wm, float ax, float ay) {
    // Calculate the dot products
    float dotHX2 = wm.x * wm.x;
    float dotHY2 = wm.y * wm.y;
    float dotHZ2 = wm.z * wm.z;

    // Square the roughness values
    float ax2 = ax * ax;
    float ay2 = ay * ay;

    // Compute the distribution D(wm)
    float denominator = (dotHX2 / ax2) + (dotHY2 / ay2) + dotHZ2;
    float D = 1.0 / (PI * ax * ay * (denominator * denominator));

    return D;
}


//===================================================================================================================
vec3 EvaluateDisneyBRDF(MaterialData material, vec3 wo, vec3 wm, vec3 wi, inout float fPdf,inout float rPdf)
{
    fPdf = 0.0f;
    rPdf = 0.0f;

    float dotNL = CosTheta(wi);
    float dotNV = CosTheta(wo);
    if(dotNL <= 0.0f || dotNV <= 0.0f) {
        return vec3(0.0f);
    }
    vec2 axy= GetAnisotropic(material.roughnessIntensity, material.anisotropic);

    float d = GgxAnisotropicD(wm, axy.x, axy.y);
    float gl = SeparableSmithGGXG1(wi, wm, axy.x, axy.y);
    float gv = SeparableSmithGGXG1(wo, wm, axy.x, axy.y);

    vec3 f = DisneyFresnel(material, wo, wm, wi);

    GgxVndfAnisotropicPdf(wi, wm, wo, axy.x, axy.y, fPdf, rPdf);

    fPdf *= (1.0f / (4 * AbsCosThetaWS(wo, wm)));
    rPdf *= (1.0f / (4 * AbsCosThetaWS(wi, wm)));
    return d * gl * gv * f / (4.0f * dotNL * dotNV);
}

//===================================================================================================================
float EvaluateDisneyRetroDiffuse(MaterialData material, vec3 wo, vec3 wm, vec3 wi){

    float dotNL = AbsCosTheta(wi);
    float dotNV = AbsCosTheta(wo);

    float roughness = material.roughnessIntensity * material.roughnessIntensity;

    float rr = 0.5f + 2.0f * dotNL * dotNL * roughness;
    float fl = SchlickWeight(dotNL);
    float fv = SchlickWeight(dotNV);

    return rr * (fl + fv + fl * fv * (rr - 1.0f));
}
//===================================================================================================================
float EvaluateDisneyDiffuse(MaterialData material, vec3 wo, vec3 wm, vec3 wi, bool thin)
{
    float dotNL = AbsCosTheta(wi);
    float dotNV = AbsCosTheta(wo);
    float fl = SchlickWeight(dotNL);
    float fv = SchlickWeight(dotNV);

    float hanrahanKrueger = 0.0f;

    //no thins implementatioon now
    if(thin && material.flatness > 0.0f) {
        float roughness = material.roughnessIntensity * material.roughnessIntensity;

        float dotHL = dot(wm, wi);
        float fss90 = dotHL * dotHL * roughness;
        float fss = Lerp(1.0f, fss90, fl) * Lerp(1.0f, fss90, fv);

        float ss = 1.25f * (fss * (1.0f / (dotNL + dotNV) - 0.5f) + 0.5f);
        hanrahanKrueger = ss;
    }

    float lambert = 1.0f;
    
    float retro = EvaluateDisneyRetroDiffuse(material, wo, wm, wi);
    //no flatness for now
    float subsurfaceApprox = Lerp(lambert, hanrahanKrueger, thin ? material.flatness : 0.0f);

    return INV_PI * ( retro + subsurfaceApprox * (1.0f - 0.5f * fl) * (1.0f - 0.5f * fv));
}

vec3 EvaluateDisneySpecTransmission(MaterialData material, vec3 wo, vec3 wm,
                                    vec3 wi, float ax, float ay, bool thin)
{
    float relativeIor = material.relativeIOR;
    float n2 = relativeIor * relativeIor;

    float absDotNL = AbsCosTheta(wi);
    float absDotNV = AbsCosTheta(wo);
    float dotHL = dot(wm, wi);
    float dotHV = dot(wm, wo);
    float absDotHL = abs(dotHL);
    float absDotHV = abs(dotHV);

    float d = GgxAnisotropicD(wm, ax, ay);
    float gl = SeparableSmithGGXG1(wi, wm, ax, ay);
    float gv = SeparableSmithGGXG1(wo, wm, ax, ay);

    float f = Dielectric(dotHV, 1.0f, material.ior);
    vec3 color;
    if(thin){
        color = sqrt(material.diffuseColor.xyz);
    }
    else{
        color = material.diffuseColor.xyz;
    }

    // Note that we are intentionally leaving out the 1/n2 spreading factor since for VCM we will be evaluating particles with
    // this. That means we'll need to model the air-[other medium] transmission if we ever place the camera inside a non-air
    // medium.
    float c = (absDotHL * absDotHV) / (absDotNL * absDotNV);
    float t = (n2 / pow(dotHL + relativeIor * dotHV, 2.0f));
    return color * c * t * (1.0f - f) * gl * gv * d;
}
//===================================================================================================================
vec3 EvaluateDisney(MaterialData material, vec3 view, vec3 light, mat3 inverseTBN,bool thin, inout float forwardPdf, inout float reversePdf)
{
    vec3 wo = normalize(inverseTBN * view);
    vec3 wi = normalize(inverseTBN * light);
    vec3 wm = normalize(wo + wi);
    float dotNV = CosTheta(wo);
    float dotNL = CosTheta(wi);

    vec3 reflectance = vec3(0.0f);
    forwardPdf = 0.0f;
    reversePdf = 0.0f;

    float pBRDF, pDiffuse, pClearcoat, pSpecTrans;
    CalculateLobePdfs(material, pBRDF, pDiffuse, pClearcoat, pSpecTrans);

    vec3 baseColor = material.diffuseColor.xyz;
    float metallic = material.metallicIntensity;
    float specTrans = material.specularTransmission;
    float roughness = material.roughnessIntensity;
    
    vec2 axy = GetAnisotropic(material.roughnessIntensity, material.anisotropic);
    
    float diffuseWeight = (1.0f - metallic) * (1.0f - specTrans);
    float transWeight   = (1.0f - metallic) * specTrans;
    // -- Clearcoat
    bool upperHemisphere = bool(dotNL > 0.0f) && bool(dotNV > 0.0f);

    if(upperHemisphere && (material.clearcoat > 0.0f)) {

        float forwardClearcoatPdfW;
        float reverseClearcoatPdfW;

        float clearcoat = EvaluateDisneyClearcoat(material.clearcoat, material.clearcoatGloss, wo, wm, wi,
                                                  forwardClearcoatPdfW, reverseClearcoatPdfW);
        reflectance += vec3(clearcoat);
        forwardPdf += pClearcoat * forwardClearcoatPdfW;
        reversePdf += pClearcoat * reverseClearcoatPdfW;
    }

//  -- Diffuse
    if(diffuseWeight > 0.0f) {
        float forwardDiffusePdfW = AbsCosTheta(wi);
        float reverseDiffusePdfW = AbsCosTheta(wo);
        float diffuse = EvaluateDisneyDiffuse(material, wo, wm, wi, thin);

        vec3 sheen = EvaluateSheen(material, wo, wm, wi);

        reflectance += diffuseWeight * (diffuse * material.diffuseColor.xyz + sheen);

        forwardPdf += pDiffuse * forwardDiffusePdfW;
        reversePdf += pDiffuse * reverseDiffusePdfW;
    }
//    // -- transmission
    if(transWeight > 0.0f) {

        // Scale roughness based on IOR (Burley 2015, Figure 15).
        float rscaled = thin ? ThinTransmissionRoughness(material.ior, material.roughnessIntensity) : material.roughnessIntensity;
        vec2 taxy = GetAnisotropic(material.roughnessIntensity, material.anisotropic);

        vec3 transmission = EvaluateDisneySpecTransmission(material, wo, wm, wi, taxy.x, taxy.y, thin);
        reflectance += transWeight * transmission;

        float forwardTransmissivePdfW;
        float reverseTransmissivePdfW;
        GgxVndfAnisotropicPdf(wi, wm, wo, taxy.x, taxy.y, forwardTransmissivePdfW, reverseTransmissivePdfW);
        float dotLH = dot(wm, wi);
        float dotVH = dot(wm, wo);
        //1.0f is the relative ior for now
        float relativeIOR = material.relativeIOR;
        forwardPdf += pSpecTrans * forwardTransmissivePdfW / (pow(dotLH + relativeIOR * dotVH, 2.0f));
        reversePdf += pSpecTrans * reverseTransmissivePdfW / (pow(dotVH + relativeIOR * dotLH, 2.0f));
    } 
    if(upperHemisphere){
        // -- specular
        float forwardMetallicPdfW;
        float reverseMetallicPdfW;
        vec3 specular = EvaluateDisneyBRDF(material, wo, wm, wi, forwardMetallicPdfW, reverseMetallicPdfW);
        reflectance += specular;
        forwardPdf += pBRDF * forwardMetallicPdfW / (4 * AbsCosThetaWS(wo, wm));
        reversePdf += pBRDF * reverseMetallicPdfW / (4 * AbsCosThetaWS(wi, wm));    
    }
    
    return reflectance;
}

//Samples
//=============================================================================================================================
void SampleDisneyBRDF(uvec2 seed, MaterialData material, vec3 v,inout vec3 l , mat3 inverseTBN, inout float forwardPdf, inout float reversePdf, inout vec3 reflectance, inout bool stopSample)
{
    vec3 wo = normalize(inverseTBN * v);

    // -- Calculate Anisotropic params
    vec2 axy;
    axy = GetAnisotropic(material.roughnessIntensity, material.anisotropic);

    // -- Sample visible distribution of normals
    float r0 = NextFloat(seed);
    float r1 = NextFloat(seed);
    vec3 wm = SampleGgxVndfAnisotropic(wo, axy.x, axy.y, r0, r1);

    // -- Reflect overwm 
    vec3 wi = normalize(reflect(wm, wo));
    if(CosTheta(wi) <= 0.0f) {
        l = vec3(0.0f);
        reflectance = vec3(0.0f);
        forwardPdf = 0.0f;
        reversePdf = 0.0f;
        stopSample = true;
        return;
    }

    // -- Fresnel term for this lobe is complicated since we're blending with both the metallic and the specularTint
    // -- parameters plus we must take the IOR into account for dielectrics
    vec3 F = DisneyFresnel(material, wo, wm, wi);

    // -- Since we're sampling the distribution of visible normals the pdf cancels out with a number of other terms.
    // -- We are left with the weight G2(wi, wo, wm) / G1(wi, wm) and since Disney uses a separable masking function
    // -- we get G1(wi, wm) * G1(wo, wm) / G1(wi, wm) = G1(wo, wm) as our weight.
    float G1v = SeparableSmithGGXG1(wo, wm, axy.x, axy.y);
    vec3 specular = G1v * F;
    
    reflectance = specular;

    l = normalize(inverse(inverseTBN) * wi);
    GgxVndfAnisotropicPdf(wi, wm, wo, axy.x, axy.y, forwardPdf, reversePdf);

    forwardPdf *= (1.0f / (4 * AbsCosThetaWS(wo, wm)));
    reversePdf *= (1.0f / (4 * AbsCosThetaWS(wi, wm)));
    stopSample = false;

}
//=============================================================================================================================
void SampleDisneyDiffuse(uvec2 seed, MaterialData material, vec3 v, bool thin, inout vec3 l , mat3 inverseTBN, inout float forwardPdf, inout float reversePdf, inout vec3 reflectance, inout bool stopSample)
{
    vec3 wo = inverseTBN * v;

    float sign = sign(CosTheta(wo));

    // -- Sample cosine lobe
    float r0 = NextFloat(seed);
    float r1 = NextFloat(seed);
    vec3 wi = sign * CosineSampleHemisphere(vec2(r0, r1));
    vec3 wm = normalize(wi + wo);

    float dotNL = CosTheta(wi);
    if(dotNL == 0.0f) {
        forwardPdf = 0.0f;
        reversePdf = 0.0f;
        reflectance = vec3(0.0f);
        l = vec3(0.0f);
        stopSample = true;
        return;
    }

    float dotNV = CosTheta(wo);

    float pdf;

    vec3 color = material.diffuseColor.xyz;

    float p = NextFloat(seed);
//    if(p <= surface.diffTrans) {
//        wi = -wi;
//        pdf = surface.diffTrans;
//        if(thin)
//        color = sqrt(color);
//        else {
            //sample.medium.phaseFunction = MediumPhaseFunction::eIsotropic;
            //sample.medium.extinction = CalculateExtinction(surface.transmittanceColor, surface.scatterDistance);
//        }
//    }
//    else {
//        pdf = (1.0f - surface.diffTrans);
//    }

    vec3 sheen = EvaluateSheen(material, wo, wm, wi);

    float diffuse = EvaluateDisneyDiffuse(material, wo, wm, wi, thin);

    reflectance = sheen + color * (diffuse / pdf);
    l = normalize(transpose(inverseTBN) * wi);
    forwardPdf = abs(dotNL) * pdf;
    reversePdf = abs(dotNV) * pdf;
    stopSample = false;
}

void SampleDisneyClearcoat(uvec2 seed, MaterialData material, vec3 v,inout vec3 l , mat3 inverseTBN, inout float forwardPdf, inout float reversePdf, inout vec3 reflectance,inout bool stopSample)
{
    vec3 wo = normalize(inverseTBN * v);
    float a = 0.25f;
    float a2 = a * a;

    float r0 = NextFloat(seed);
    float r1 = NextFloat(seed);
    float cosTheta = sqrt(max((1.0f - pow(1.0f - r0, a2)) / (1.0f - a2), 0.0f));
    float sinTheta = sqrt(max(1.0f - cosTheta * cosTheta, 0.0f));
    float phi = (PI*2) * r1;

    vec3 wm = vec3(sinTheta * cos(phi), cosTheta, sinTheta * sin(phi));
    if(dot(wm, wo) < 0.0f) {
        wm = -wm;
    }

    vec3 wi = reflect(wm, wo);
    if(dot(wi, wo) < 0.0f) {
        stopSample = true;
        return;
    }

    float clearcoatWeight = material.clearcoat;
    float clearcoatGloss = material.clearcoatGloss;

    float dotNH = CosTheta(wm);
    float dotLH = dot(wm, wi);

    float d = GTR1(abs(dotNH), Lerp(0.1f, 0.001f, clearcoatGloss));
    float f = Schlick(0.04f, dotLH);
    float g = SeparableSmithGGXG1(wi, 0.25f) * SeparableSmithGGXG1(wo, 0.25f);

    float fPdf = d / (4.0f * dot(wo, wm));

    reflectance = vec3(0.25f * clearcoatWeight * g * f * d) / fPdf;
    l = normalize(transpose(inverseTBN) * wi);
    forwardPdf = fPdf;
    reversePdf = d / (4.0f * dot(wi, wm));
    stopSample = false;

}

void SampleDisneySpecTransmission(uvec2 seed, MaterialData material, vec3 v, bool thin, inout vec3 l, mat3 inverseTBN, inout float forwardPdf,
inout float reversePdf, inout vec3 reflectance, inout bool stopSample)
{
    vec3 wo = normalize(inverseTBN * v);
    if(CosTheta(wo) == 0.0) {
        forwardPdf = 0.0f;
        reversePdf = 0.0f;
        reflectance = vec3(0.0f);
        l = vec3(0.0f);
        stopSample = true;
        return;
    }

    // -- Scale roughness based on IOR
    float rscaled = thin ? ThinTransmissionRoughness(material.ior, material.roughnessIntensity) : material.roughnessIntensity;

    vec2 taxy = GetAnisotropic(rscaled, material.anisotropic);

    // -- Sample visible distribution of normals
    float r0 = NextFloat(seed);
    float r1 = NextFloat(seed);
    vec3 wm = SampleGgxVndfAnisotropic(wo, taxy.x, taxy.y, r0, r1);

    float dotVH = dot(wo, wm);
    if(wm.y < 0.0f) {
        dotVH = -dotVH;
    }

    float ni = wo.y > 0.0f ? 1.0f : material.ior;
    float nt = wo.y > 0.0f ? material.ior : 1.0f;
    float relativeIOR = ni / nt;

// -- Disney uses the full dielectric Fresnel equation for transmission. We also importance sample F to switch between
// -- refraction and reflection at glancing angles.
    float F = Dielectric(dotVH, 1.0f, material.ior);

// -- Since we're sampling the distribution of visible normals the pdf cancels out with a number of other terms.
// -- We are left with the weight G2(wi, wo, wm) / G1(wi, wm) and since Disney uses a separable masking function
// -- we get G1(wi, wm) * G1(wo, wm) / G1(wi, wm) = G1(wo, wm) as our weight.
    float G1v = SeparableSmithGGXG1(wo, wm, taxy.x, taxy.y);

    float pdf;

    vec3 wi;
    if(NextFloat(seed) <= F) {
        wi = normalize(reflect(wm, wo));

        reflectance = G1v * material.diffuseColor.xyz;

        float jacobian = (4 * AbsCosThetaWS(wo, wm));
        pdf = F / jacobian;
    }
    else {
        if(thin) {
            // -- When the surface is thin so it refracts into and then out of the surface during this shading event.
            // -- So the ray is just reflected then flipped and we use the sqrt of the surface color.
            wi = reflect(wm, wo);
            wi.y = -wi.y;
            reflectance = G1v * sqrt(material.diffuseColor.xyz);

// -- Since this is a thin surface we are not ending up inside of a volume so we treat this as a scatter event.
        }
        else {
//            if(Transmit(wm, wo, relativeIOR, wi)) {
//             sample.medium.phaseFunction = dotVH > 0.0f ? MediumPhaseFunction::eIsotropic : MediumPhaseFunction::eVacuum;
//            sample.medium.extinction = CalculateExtinction(surface.transmittanceColor, surface.scatterDistance);
//            }
//            else {
//            }
            wi = reflect(wm, wo);
            reflectance = G1v * material.diffuseColor.xyz;
        }

        wi = normalize(wi);

        float dotLH = abs(dot(wi, wm));
        float jacobian = dotLH / (pow(dotLH + material.relativeIOR * dotVH, 2.0f));
        pdf = (1.0f - F) / jacobian;
    }

    if(CosTheta(wi) == 0.0f) {
        forwardPdf = 0.0f;
        reversePdf = 0.0f;
        reflectance = vec3(0.0f);
        l = vec3(0.0f);
        stopSample = true;
        return;
    }

    if(material.roughnessIntensity < 0.01f) {
//        sample.flags |= SurfaceEventFlags::eDiracEvent;
    }

    // -- calculate pdf terms
    GgxVndfAnisotropicPdf(wi, wm, wo, taxy.x, taxy.y,forwardPdf, reversePdf);
    forwardPdf *= pdf;
    reversePdf *= pdf;
    // -- convert wi back to world space
    l = normalize(inverse(inverseTBN) * wi);
    stopSample = false;
}






void SampleDisney(uvec2 seed, MaterialData material, bool thin, vec3 v,inout vec3 l, mat3 inverseTBN, inout float forwardPdf, inout float reversePdf,inout vec3 reflectance, inout bool stopSample)
{
    float pSpecular;
    float pDiffuse;
    float pClearcoat;
    float pTransmission;
    CalculateLobePdfs(material, pSpecular, pDiffuse, pClearcoat, pTransmission);

    float pLobe = 0.0f;
    float p = NextFloat(seed);
    if(p <= pSpecular) {
        SampleDisneyBRDF(seed, material, v, l, inverseTBN, forwardPdf, reversePdf, reflectance, stopSample);
        pLobe = pSpecular;
    }
    else if(p > pSpecular && p <= (pSpecular + pClearcoat)) {
        SampleDisneyClearcoat(seed, material, v, l, inverseTBN, forwardPdf, reversePdf, reflectance, stopSample);
        pLobe = pClearcoat;
    }
    else if(p > pSpecular + pClearcoat && p <= (pSpecular + pClearcoat + pDiffuse)) {
        SampleDisneyDiffuse(seed, material, v, thin, l, inverseTBN, forwardPdf, reversePdf, reflectance, stopSample);
        pLobe = pDiffuse;
    }
    else if(pTransmission >= 0.0f) {
        SampleDisneySpecTransmission(seed, material, v, thin, l, inverseTBN, forwardPdf, reversePdf, reflectance, stopSample);
        pLobe = pTransmission;
    }
    else {
        // -- Make sure we notice if this is occurring.
        reflectance = vec3(1000000.0f, 0.0f, 0.0f);
        forwardPdf = 0.000000001f;
        reversePdf = 0.000000001f;
    }

    if(pLobe > 0.0f) {
        reflectance = reflectance * (1.0f / pLobe);
        forwardPdf *= pLobe;
        reversePdf *= pLobe;
    }
//     debuging
//    vec3 wo = inverseTBN * v;
//    float sign = sign(CosTheta(wo));
//    vec3 wi = sign * CosineSampleHemisphere(NextVec2(seed));
//    l = inverse(inverseTBN) * wi;
}





/////////////////////////////Different implementation//////////////////////////////////////////////////// 

vec3 EvalDielectricReflection(in MaterialData material, vec3 V, vec3 N, vec3 L, vec3 H, inout float pdf)
{
    pdf = 0.0;
    if (dot(N, L) <= 0.0){
        return vec3(0.0);
    }

    float eta = material.ior;

    float F = Dielectric(dot(V, H), 1.0f, eta);
    float D = GTR2(dot(N, H), material.roughnessIntensity);

    pdf = D * dot(N, H) * F / (4.0 * abs(dot(V, H)));

    float G = SmithGGX(abs(dot(N, L)), material.roughnessIntensity) * SmithGGX(abs(dot(N, V)), material.roughnessIntensity);

    return material.diffuseColor.xyz * F * D * G;
}


vec3 EvalDielectricRefraction(in MaterialData material, vec3 V, vec3 N, vec3 L, vec3 H, inout float pdf)
{
    float eta = material.ior;

    pdf = 0.0;
    if (dot(N, L) >= 0.0)
    return vec3(0.0);

    float F = Dielectric(abs(dot(V, H)), 1.0f, eta);
    float D = GTR2(dot(N, H), material.roughnessIntensity);

    float denomSqrt = dot(L, H) + dot(V, H) * eta;
    pdf = D * dot(N, H) * (1.0 - F) * abs(dot(L, H)) / (denomSqrt * denomSqrt);

    float G = SmithGGX(abs(dot(N, L)), material.roughnessIntensity) * SmithGGX(abs(dot(N, V)), material.roughnessIntensity);

    return material.diffuseColor.xyz * (1.0 - F) * D * G * abs(dot(V, H)) * abs(dot(L, H)) * 4.0 * eta * eta / (denomSqrt * denomSqrt);
}

vec3 EvalSpecular(in MaterialData material, in vec3 Cspec0, vec3 V, vec3 N, vec3 L, vec3 H, inout float pdf)
{
    pdf = 0.0;
    if (dot(N, L) <= 0.0){
        return vec3(0.0);
    }

    float D = GTR2(dot(N, H), material.roughnessIntensity);
    pdf = D * dot(N, H) / (4.0 * dot(V, H));

    float FH = SchlickFresnel(dot(L, H));
    vec3 F = mix(Cspec0, vec3(1.0), FH);
    float G = SmithGGX(abs(dot(N, L)), material.roughnessIntensity) * SmithGGX(abs(dot(N, V)), material.roughnessIntensity);

    return F * D * G;
}

vec3 EvalClearcoat(in MaterialData material, vec3 V, vec3 N, vec3 L, vec3 H, inout float pdf)
{
    pdf = 0.0;
    if (dot(N, L) <= 0.0)
    return vec3(0.0);

    float D = GTR1(dot(N, H), mix(0.1, 0.001, material.clearcoatGloss));
    pdf = D * dot(N, H) / (4.0 * dot(V, H));

    float FH = SchlickFresnel(dot(L, H));
    float F = mix(0.04, 1.0, FH);
    float G = SmithGGX(dot(N, L), 0.25) * SmithGGX(dot(N, V), 0.25);

    return vec3(0.25 * material.clearcoat * F * D * G);
}


vec3 EvalDiffuse(in MaterialData material, in vec3 Csheen, vec3 V, vec3 N, vec3 L, vec3 H, inout float pdf)
{
    pdf = 0.0;
    if (dot(N, L) <= 0.0)
    return vec3(0.0);

    pdf = dot(N, L) * (1.0 / PI);

    // Diffuse
    float FL = SchlickFresnel(dot(N, L));
    float FV = SchlickFresnel(dot(N, V));
    float FH = SchlickFresnel(dot(L, H));
    float Fd90 = 0.5 + 2.0 * dot(L, H) * dot(L, H) * material.roughnessIntensity;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    float Fss90 = dot(L, H) * dot(L, H) * material.roughnessIntensity;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (dot(N, L) + dot(N, V)) - 0.5) + 0.5);

    vec3 Fsheen = FH * material.sheen * Csheen;

    return ((1.0 / PI) * mix(Fd, ss, material.subSurface) * material.diffuseColor.xyz + Fsheen) * (1.0 - material.metallicIntensity);
}
vec3 DisneyEval(MaterialData material,  vec3 V,vec3 L, vec3 N, inout float pdf)
{
    float eta = 1.0f;

    vec3 H;
    bool refl = bool(dot(N, L) > 0.0);

    if (refl){
        H = normalize(L + V);
    }
    else{
        H = normalize(L + V * eta);
    }

    if (dot(V, H) < 0.0){
        H = -H;
    }
    float diffuseRatio = 0.5 * (1.0 - material.metallicIntensity);
    float primarySpecRatio = 1.0 / (1.0 + material.clearcoat);
    float transWeight = (1.0 - material.metallicIntensity) * material.specularTransmission;
    vec3 brdf = vec3(0.0);
    vec3 bsdf = vec3(0.0);
    float brdfPdf = 0.0;
    float bsdfPdf = 0.0;
    if (transWeight > 0.0)
    {
        // Reflection
        if (refl)
        {
            bsdf = EvalDielectricReflection(material, V, N, L, H, bsdfPdf);
        }
        else // Transmission
        {
            bsdf = EvalDielectricRefraction(material, V, N, L, H, bsdfPdf);
        }
    }
    float m_pdf;

    if (transWeight < 1.0)
    {
        vec3 Cdlin = material.diffuseColor.xyz;
        float Cdlum = 0.3 * Cdlin.x + 0.6 * Cdlin.y + 0.1 * Cdlin.z; // luminance approx.

        vec3 Ctint = Cdlum > 0.0 ? Cdlin / Cdlum : vec3(1.0f); // normalize lum. to isolate hue+sat
        vec3 Cspec0 = mix(material.diffuseColor.w * 0.08 * mix(vec3(1.0), Ctint, material.specularTint), Cdlin, material.metallicIntensity);
        vec3 Csheen = mix(vec3(1.0), Ctint, material.sheenTint);

        // Diffuse
        brdf += EvalDiffuse(material, Csheen, V, N, L, H, m_pdf);
        brdfPdf += m_pdf * diffuseRatio;

        // Specular
        brdf += EvalSpecular(material, Cspec0, V, N, L, H, m_pdf);
        brdfPdf += m_pdf * primarySpecRatio * (1.0 - diffuseRatio);

        // Clearcoat
        brdf += EvalClearcoat(material, V, N, L, H, m_pdf);
        brdfPdf += m_pdf * (1.0 - primarySpecRatio) * (1.0 - diffuseRatio);
    }
    pdf = mix(brdfPdf, bsdfPdf, transWeight);

    return mix(brdf, bsdf, transWeight);
}
vec3 DisneySample(in MaterialData material,uvec2 seed,vec3 V, vec3 N, inout vec3 L, inout float pdf)
{
    pdf = 0.0;
    vec3 f = vec3(0.0);

    float diffuseRatio = 0.5 * (1.0 - material.metallicIntensity);
    float transWeight = (1.0 - material.metallicIntensity) * material.specularTransmission;

    vec3 Cdlin = material.diffuseColor.xyz;
    float Cdlum = 0.3 * Cdlin.x + 0.6 * Cdlin.y + 0.1 * Cdlin.z; // luminance approx.

    vec3 Ctint = Cdlum > 0.0 ? Cdlin / Cdlum : vec3(1.0f); // normalize lum. to isolate hue+sat
    vec3 Cspec0 = mix(material.diffuseColor.w * 0.08 * mix(vec3(1.0), Ctint, material.specularTint), Cdlin, material.metallicIntensity);
    vec3 Csheen = mix(vec3(1.0), Ctint, material.sheenTint);
    float eta = material.ior;

    vec3 T, B;

    CreateOrthonormalBasis(N, T, B);
    mat3 frame = mat3(T, B, N);

    float r1 = NextFloat(seed);
    float r2 = NextFloat(seed);

    if (NextFloat(seed) < transWeight)
    {
        vec3 H = ImportanceSampleGTR2(material.roughnessIntensity, r1, r2);
        H = frame * H;

        if (dot(V, H) < 0.0)
        H = -H;

        vec3 R = reflect(-V, H);
        float F = Dielectric(abs(dot(R, H)), material.ior, eta);

        // Reflection/Total internal reflection
        if (r2 < F)
        {
            L = normalize(R);
            f = EvalDielectricReflection(material, V, N, L, H, pdf);
        }
        else // Transmission
        {
            L = normalize(refract(-V, H, eta));
            f = EvalDielectricRefraction(material, V, N, L, H, pdf);
        }

        f *= transWeight;
        pdf *= transWeight;
    }
    else
    {

        if (NextFloat(seed) < diffuseRatio)
        {
            L = CosineSampleHemisphere(NextVec2(seed));
            L = frame * L;

            vec3 H = normalize(L + V);

            f = EvalDiffuse(material, Csheen, V, N, L, H, pdf);
            pdf *= diffuseRatio;
        }
        else // Specular
        {
            float primarySpecRatio = 1.0 / (1.0 + material.clearcoat);

            // Sample primary specular lobe
            if (NextFloat(seed) < primarySpecRatio)
            {
                vec3 H = ImportanceSampleGTR2(material.roughnessIntensity, r1, r2);
                H = frame * H;

                if (dot(V, H) < 0.0)
                H = -H;

                L = normalize(reflect(-V, H));

                f = EvalSpecular(material, Cspec0, V, N, L, H, pdf);
                pdf *= primarySpecRatio * (1.0 - diffuseRatio);
            }
            else // Sample clearcoat lobe
            {
                vec3 H = ImportanceSampleGTR1(mix(0.1, 0.001, material.clearcoatGloss), r1, r2);
                H = frame * H;

                if (dot(V, H) < 0.0)
                H = -H;

                L = normalize(reflect(-V, H));

                f = EvalClearcoat(material, V, N, L, H, pdf);
                pdf *= (1.0 - primarySpecRatio) * (1.0 - diffuseRatio);
            }
        }
    }
    f *= (1.0 - transWeight);
    pdf *= (1.0 - transWeight);
    return f;
}
#endif 