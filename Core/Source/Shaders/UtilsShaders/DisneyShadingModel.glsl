
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
    float dotHV = abs(dot(wm, wo));

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
    float f = SchlickF(0.04f, dotHL);
    float gl = SeparableSmithGGXG1(wi, 0.25f);
    float gv = SeparableSmithGGXG1(wo, 0.25f);

    fPdfW = d / (4.0f * AbsCosThetaWS(wo,wm));
    rPdfW = d / (4.0f * AbsCosThetaWS(wi,wm));

    return 0.25f * clearcoat   ;
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
//
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
//     -- transmission
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
    if(upperHemisphere) {
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
    vec3 wi = normalize(reflect(-wo,wm));
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
    
    vec3 specular =  F * G1v;

    float p1, p2;
//    vec3 specular = EvaluateDisneyBRDF(material, wo, wm, wi, p1, p2);;
    reflectance = specular;

    l = normalize(transpose(inverseTBN) * wi);
    GgxVndfAnisotropicPdf(wi, wm, wo, axy.x, axy.y, forwardPdf, reversePdf);
    
    forwardPdf *= (1.0f / (4 * AbsCosThetaWS(wo, wm)));
    reversePdf *= (1.0f / (4 * AbsCosThetaWS(wi, wm)));
    stopSample = false;

}
//=============================================================================================================================
void SampleDisneyDiffuse(uvec2 seed, MaterialData material, vec3 v, bool thin, inout vec3 l , mat3 inverseTBN, inout float forwardPdf, inout float reversePdf, inout vec3 reflectance, inout bool stopSample)
{
    vec3 wo = inverseTBN * v;

    float coSign = sign(CosTheta(wo));

    // -- Sample cosine lobe
    float r0 = NextFloat(seed);
    float r1 = NextFloat(seed);
    vec3 wi = CosineSampleHemisphere(vec2(r0, r1));
    
    vec3 wm = normalize(wi + wo);

    float dotNL = CosTheta(wi);
    if(dotNL == 0.0f) {
        forwardPdf = 0.0f;
        reversePdf = 0.0f;
        reflectance = vec3(0.0f, 0.0f, 0.0f);
        l = vec3(0.0f);
        stopSample = true;
        return;
    }

    float dotNV = CosTheta(wo);

    float pdf;

    vec3 color = material.diffuseColor.xyz;

    float p = NextFloat(seed);
    vec3 extinction = vec3(1.0f);
    if(p <= material.diffTransmission) {
        wi = -wi;
        pdf = material.diffTransmission;
        if(thin){
            color = sqrt(color);
        }
        else {
            //sample.medium.phaseFunction = MediumPhaseFunction::eIsotropic;
            extinction = CalculateExtinction(material.transColor, material.scatterDistance);
        }
    }
    else {
        pdf = (1.0f - material.diffTransmission);
    }

    vec3 sheen = EvaluateSheen(material, wo, wm, wi);

    float diffuse = EvaluateDisneyDiffuse(material, wo, wm, wi, thin);

    reflectance = sheen + color * extinction * (diffuse / pdf);
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

    vec3 wi = reflect(-wo, wm);
    if(dot(wi, wo) < 0.0f) {
        stopSample = true;
        return;
    }

    float clearcoatWeight = material.clearcoat;
    float clearcoatGloss = material.clearcoatGloss;

    float dotNH = CosTheta(wm);
    float dotLH = dot(wm, wi);

    float d = GTR1(abs(dotNH), Lerp(0.1f, 0.001f, clearcoatGloss));
    float f = SchlickF(0.04f, dotLH);
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
        wi = normalize(reflect(-wo,wm));

        reflectance = G1v * material.diffuseColor.xyz;

        float jacobian = (4 * AbsCosThetaWS(wo, wm));
        pdf = F / jacobian;
    }
    else {
        if(thin) {
            // -- When the surface is thin so it refracts into and then out of the surface during this shading event.
            // -- So the ray is just reflected then flipped and we use the sqrt of the surface color.
            wi = normalize(reflect(-wo,wm));
            wi.y = -wi.y;
            reflectance = G1v * sqrt(material.diffuseColor.xyz);

// -- Since this is a thin surface we are not ending up inside of a volume so we treat this as a scatter event.
        }
        else {
            vec3 extinction = vec3(1.0f);
            if(Transmit(wm, wo, relativeIOR, wi)) {
               wi = normalize(refract(-wo, wm, relativeIOR));
               extinction = CalculateExtinction(material.transColor, material.scatterDistance);
//               reflectance = G1v * material.diffuseColor.xyz * extinction;
                
            }
            else {
                wi = reflect(-wo, wm);
//                reflectance = G1v * material.diffuseColor.xyz;
            }

            reflectance = G1v * material.diffuseColor.xyz * extinction;
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
//         -- Make sure we notice if this is occurring.
        reflectance = vec3(1000000.0f, 0.0f, 0.0f);
        forwardPdf = 0.000000001f;
        reversePdf = 0.000000001f;
    }

    if(pLobe > 0.0f) {
        reflectance = reflectance * (1.0f / pLobe);
        forwardPdf *= pLobe;
        reversePdf *= pLobe;
    }

}

#endif 