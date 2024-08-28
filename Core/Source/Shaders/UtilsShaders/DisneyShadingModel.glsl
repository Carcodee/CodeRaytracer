
// w out is view dir, could change later
float CosthetaTangent(vec3 v1, vec3 v2){
    return max(dot(v1, v2), 0.001);
}

float Luminance(vec3 baseCol){
    return 0.2126*baseCol.r +0.7152* baseCol.g +0.0722* baseCol.b;
}
vec3 Ctint (vec3 baseCol){
    float luminance= Luminance(baseCol);
    if(luminance>0){
        return baseCol / luminance;
    }else{
        return vec3(1);
    }
}
float Ro(float n){
    float ro = pow((n - 1.0f) / (n + 1.0f), 2.0f);
    return ro;
}
//difuse
float FD90(float roughness, vec3 halfway, vec3 normal){
    float costhetaTangent = CosthetaTangent(halfway, normal);
    float secondSumPart = (2.0f*roughness)*pow(costhetaTangent, 2.0);
    return 0.5f + secondSumPart; 
};
float FSS90(float roughness, vec3 halfway, vec3 view){
    float costhetaTangent = CosthetaTangent(halfway, view);
    float result = roughness * pow(costhetaTangent, 2.0f);
    return result;
}
float FSSW(vec3 halfway, vec3 view, vec3 w, vec3 normal, float roughness){
    float fss90= FSS90(roughness, halfway, view);
    float costhetaTangent = CosthetaTangent(w, normal);
    float result = (1.0f + ((fss90 - 1)*(pow(1 - costhetaTangent, 5.0f))));
    return result;
}
float FDDiffuse(float fd90, float costheta){
    float result = 1.0f + ((fd90 - 1.0f) * pow(1 - costheta, 2.0f));
    return result;
}

vec3 DisneyFS(vec3 baseCol, vec3 normal, vec3 halfway, vec3 view, vec3 lightDir, float rougness){
    
    vec3 colPart = (baseCol* 1.25)/3.14;
    float FSSWin = FSSW(halfway, view, lightDir, normal, rougness);
    float FSSWOut = FSSW(halfway, view, view, normal, rougness);
    float costhetaLight = CosthetaTangent(lightDir, normal);
    float costhetaView = CosthetaTangent(view, normal);
    
    float division = (1.0f/(costhetaLight + costhetaView)) - 0.5f;
    
    vec3 fSubsurface = colPart* ((FSSWin* FSSWOut*division) + 0.5f) * costhetaView;
    return fSubsurface;
}
vec3 DisneyFD(vec3 baseCol, vec3 normal, vec3 halfway, vec3 view, vec3 lightDir, float rougness){
    vec3 lambert = baseCol/3.14f;
    float costhetaLight = CosthetaTangent(lightDir, normal);
    float costhetaView = CosthetaTangent(view, normal);
    float fd90 = FD90(rougness, halfway, normal);
    float fdView = FDDiffuse(fd90, costhetaView);
    float fdLight = FDDiffuse(fd90, costhetaLight);
    vec3 result = lambert * fdView * fdLight * costhetaView;
    
    return result;
};

vec3 DisneyDiffuse(vec3 baseCol, vec3 normal, vec3 halfway, vec3 view, vec3 lightDir, float rougness, float subsurface){
    vec3 fd = DisneyFD(baseCol, normal, halfway, view, lightDir, rougness);
    vec3 fs = DisneyFS(baseCol, normal, halfway, view, lightDir, rougness);
    vec3 result = ((1.0f - subsurface)* fd) + (subsurface*fs);
    return result;
}

void GetAlphas(out float alphaX, out float alphaY, float anisotropic, float roughness){
    float aspect = sqrt(1.0f - (0.9f*(anisotropic)));
    alphaX = max(pow(roughness, 2.0f)/aspect,0.001f);
    alphaY = max(pow(roughness, 2.0f)*aspect,0.001f);
}

//metallic
vec3 Fm (vec3 baseCol, vec3 view, vec3 halfway){
    vec3 fm = baseCol +((vec3(1.0f) - baseCol)*(1 - pow(CosthetaTangent(halfway, view), 5.0f)));
    return fm;
}

vec3 Ks(vec3 baseCol, float specularTint){
    vec3 cTint= Ctint(baseCol);
    return (1.0f - specularTint) + (specularTint * cTint);
}
vec3 Co(vec3 baseCol, float specularTint, float specular, float metallic, float refraction){
    float ro = Ro(refraction);
    vec3 ks = Ks(baseCol, specularTint);
    return (specular * ro * (1 - metallic) * ks) + (metallic * baseCol);
    
}
vec3 FmHat (vec3 baseCol, vec3 view, vec3 halfway, float specularTint, float specular, float metallic, float refraction){
    vec3 co = Co(baseCol, specularTint, specular, metallic, refraction);
    vec3 result = co + ((vec3(1.0f) - co) * pow((1 - CosthetaTangent(halfway, view)), 5.0f));
    return result;
}

//hl is halfway in tangent space
float Dm(float roughness,float anisotropic, vec3 hl, float alphaX, float alphaY){
    float hlpowX = pow(hl.x, 2.0f);
    float hlpowY = pow(hl.y, 2.0f);
    float hlpowZ = pow(hl.z, 2.0f);
    float alphas = 3.14 * alphaX * alphaY;
    float denomParentesis = pow((hlpowX/pow(alphaX, 2)) + (hlpowY/pow(alphaY, 2)) + (hlpowZ), 2.0f); 
    float denom = alphas*denomParentesis;
    return 1/denom;
};
//wl can be wo or wi
float GDenom(float roughness, float anisotropic, vec3 wl, float alphaX, float alphaY){
    float wlpowX = pow(wl.x * alphaX, 2.0f);
    float wlpowY = pow(wl.y * alphaY, 2.0f);
    float wlpowZ = pow(wl.z, 2.0f);
    float sqrtPart = 1 + ((wlpowX + wlpowY)/wlpowZ);
    sqrtPart = sqrt(sqrtPart) - 1.0f;
    return sqrtPart/2.0f;
}
float Gw(float roughness, float anisotropic, vec3 wl, float alphaX,float alphaY){
    float gDenom = GDenom(roughness, anisotropic, wl, alphaX, alphaY);
    return 1 / (1 + gDenom);
}
float Gm(float roughness, float anisotropic, vec3 wlIn, vec3 wlOut, float alphaX, float alphaY){
    float GIn =Gw(roughness, anisotropic, wlIn, alphaX, alphaY);
    float GOut = Gw(roughness, anisotropic, wlOut, alphaX, alphaY);
    return GIn*GOut;
}

vec3 DisneyMetallic(vec3 baseCol, float specularTint, float roughness, float anisotropic, float metallic,float refraction, float specular, vec3 halfway, vec3 view, vec3 lightDir, vec3 normal,vec3 hl,vec3 wlIn, vec3 wlOut){
    float alphaX;
    float alphaY;
    GetAlphas(alphaX, alphaY, anisotropic, roughness);
    vec3 fm =FmHat(baseCol, view, halfway, specularTint, specular, metallic, refraction);
    float dm = Dm(roughness, anisotropic, hl, alphaX, alphaY);
    //light dir and view maybe are both wl terms
    float gm= Gm(roughness, anisotropic, lightDir, view, alphaX, alphaY);
    vec3 num = fm*gm*dm;
    float denom = 4* CosthetaTangent(lightDir, normal);
    return num/denom;
} 

// clearcoat

float Fc (vec3 halfway, vec3 view, float refraction){
    float ro = Ro(refraction);
    float costhetaTangent= CosthetaTangent(halfway, view);
    float powPart= (1- ro) * pow(1 - costhetaTangent, 5.0f);
    float result = ro + powPart;
    return result;
}
float GetClearcoatGloss(float clearcoatGlossParam){
    float result = ((1.0f - clearcoatGlossParam) * 0.1f) + (clearcoatGlossParam* 0.001);
    return result;
};
float Dc(float clearcoatGlossParam, vec3 hl){
    float clearcoatGloss = GetClearcoatGloss(clearcoatGlossParam);
    float clearcoatGlossPow = pow(clearcoatGloss, 2.0f);
    float hlzPow = pow(hl.z, 2.0f);

    float num = clearcoatGlossPow - 1.0f;
    float denom = (3.14 * log(clearcoatGlossPow)) * (1.0f + ((clearcoatGlossPow - 1)*(hlzPow)));
    return num / denom;
};
 
float AcW(vec3 wl){
    float wlpowX = pow(wl.x * 0.25f, 2.0f);
    float wlpowY = pow(wl.y * 0.25f, 2.0f);
    float wlpowZ = pow(wl.z, 2.0f);
    float sqroot =sqrt(1 + ((wlpowX*wlpowY)/wlpowZ));
    float num = (sqroot-1);
    return num/2;
}
float GcW(float acw){
    return 1 / (1 + acw);
}
float Gc(vec3 wlIn, vec3 wlOut){
    float AcIn = AcW(wlIn);
    float AcOut = AcW(wlOut);
    float GcIn= GcW(AcIn);
    float GcOut= GcW(AcOut);
    return GcIn*GcOut;
}

float DisneyClearcoat(vec3 halfway, vec3 view, vec3 hl, vec3 wlIn, vec3 wlOut, vec3 normal, vec3 lightDir, float clearcloatGlossParam, float refraction){
    float fc = Fc(halfway, view, refraction);
    float dc = Dc(clearcloatGlossParam, hl);
    float gc = Gc(wlIn, wlOut);
    float num = fc*dc*gc;
    float denom = 4.0f * CosthetaTangent(lightDir, normal);
    return num/denom;
}

//glass

float Rs(vec3 halfway, vec3 wIn, vec3 wOut, float refraction){
    float num = CosthetaTangent(halfway, wIn) - (refraction * CosthetaTangent(halfway, wOut));
    float denom = CosthetaTangent(halfway, wIn) + (refraction * CosthetaTangent(halfway, wOut));
    return num/denom;
}
float Rp(vec3 halfway, vec3 wIn, vec3 wOut, float refraction){
    float num = (refraction *CosthetaTangent(halfway, wIn)) - CosthetaTangent(halfway, wOut);
    float denom = (refraction *CosthetaTangent(halfway, wIn)) + CosthetaTangent(halfway, wOut);
    return num/denom;
}
float Fg(vec3 halfway, vec3 wIn, vec3 wOut, float refraction){
    float rs = Rs(halfway, wIn, wOut, refraction);
    float rp = Rp(halfway, wIn, wOut, refraction);
    return (1/2) * (pow(rs, 2.0f) + pow(rp, 2.0f));
}
vec3 DisneyGlass(vec3 baseCol, float roughness, float anisotropic, float refraction, vec3 halfway, vec3 view, vec3 lightDir, vec3 normal,vec3 hl,vec3 wlIn, vec3 wlOut){
    float alphaX;
    float alphaY;
    GetAlphas(alphaX, alphaY, anisotropic, roughness);
    float fg = Fg(halfway, wlIn, wlOut, refraction);
    float dg = Dm(roughness,anisotropic, hl, alphaX, alphaY);
    float gg = Gm(roughness, anisotropic, wlIn, wlOut, alphaX, alphaY);
    float costhetaLight = CosthetaTangent(lightDir, normal);
    float costhetaView = CosthetaTangent(view, normal);
    vec3 result;
    if(costhetaLight * costhetaView > 0){
        vec3 num = baseCol * fg * dg * gg;
        float denom = 4 * costhetaLight;
        result = num/denom;
    }else{
        vec3 num = sqrt(baseCol) * (1 - fg)* dg * gg * CosthetaTangent(halfway, lightDir)*CosthetaTangent(halfway, view);
        float denom = costhetaLight * pow(dot(halfway, lightDir) + (refraction * dot(halfway, view)),2.0f);
        result= num/denom;
    }
    return result;
}

//sheen

vec3 Csheen(vec3 baseCol, float sheenTint){
    vec3 cTint = Ctint(baseCol);
    vec3 result = (1 - sheenTint) + (sheenTint * cTint);
    return result;
}
vec3 DisneySheen(vec3 baseCol, float sheenTint, vec3 halfway, vec3 view, vec3 normal){
    vec3 cSheen = Csheen(baseCol, sheenTint);
    float costhetaHalfway = CosthetaTangent(halfway, view);
    float costhetaView = CosthetaTangent(view, normal);
    vec3 result = cSheen * pow((1 - costhetaHalfway), 5.0f) * costhetaView;
    return result;
}

//test 

//good for spheres but is not working
vec3 GetDisneyBSDF(vec3 baseCol, float roughness, float subSurface, float anisotropic,float clearcoatParam, 
                   float clearcoatGlossParam, float metallic, float specular, float specularTint,
                   float specularTransmission, float sheenTint, float sheen, float refraction,
                   vec3 halfway, vec3 view, vec3 lightDir, vec3 normal,vec3 hl,vec3 wlIn, vec3 wlOut){
    vec3 fd = DisneyDiffuse(baseCol, normal, halfway, view, lightDir, roughness, subSurface);
    vec3 fm = DisneyMetallic(baseCol, specularTint, roughness, anisotropic, metallic,refraction, specular, halfway, view, lightDir, normal,hl,wlIn, wlOut);
    float fc = DisneyClearcoat(halfway, view, hl, wlIn, wlOut, normal, lightDir, clearcoatGlossParam, refraction);
    vec3 fs = DisneySheen(baseCol, sheenTint, halfway, view, normal);
    vec3 fg = DisneyGlass(baseCol, roughness, anisotropic, refraction, halfway, view, lightDir, normal,hl,wlIn, wlOut);
    
    
    vec3 val = ((1.0f - specularTransmission) * (1 - metallic) * fd) +
               ((1.0f - metallic) * sheen * fs) +
               ((1.0f - specularTransmission) * (1 - metallic) * fm) +
               ((0.25f * clearcoatParam) * fc);
//               ((1.0f - metallic) * specularTransmission * fg);

    return val;
}