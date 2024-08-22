
#define N 1.5 
// w out is view dir, could change later
float CosthetaTangent(vec3 v1, vec3 v2){
    return max(dot(v1, v2), 0.001);
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
    float FD90Val = FD90(rougness, halfway, normal);
    float fdPart = FD90Val - 1.0f;
    float viewPart = pow(1.0f - costhetaView, 5.0f);
    float lightPart = pow(1.0f - costhetaLight, 5.0f);
    vec3 fd = lambert*(1.0f + (fdPart * lightPart))*(1.0f + (fdPart * viewPart));
    
    return fd;
};

vec3 DisneyDiffuse(vec3 baseCol, vec3 normal, vec3 halfway, vec3 view, vec3 lightDir, float rougness, float subsurface){
    vec3 fd = DisneyFD(baseCol, normal, halfway, view, lightDir, rougness);
    vec3 fs = DisneyFS(baseCol, normal, halfway, view, lightDir, rougness);
    vec3 result = ((1.0f - subsurface)* fd) + (subsurface*fs);
    return result;
}

void GetAlphas(out float alphaX, out float alphaY, float anisotropic, float roughness){
    float aspect = sqrt(1.0f - (0.9f*(max(anisotropic, 0.001f))));
    alphaX = max(pow(roughness, 2)/aspect,0.001f);
    alphaY = max(pow(roughness, 2)*aspect,0.001f);
}

//metallic
vec3 Fm (vec3 baseCol, vec3 view, vec3 halfway){
    vec3 fm = baseCol +((vec3(1.0f) - baseCol)*(1 - pow(CosthetaTangent(halfway, view), 5.0f)));
    return fm;
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
    float sqrtPart = (wlpowX + wlpowY)/wlpowZ;
    sqrtPart = sqrt(1.0f + sqrtPart) - 1.0f;
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

vec3 DisneyMetallic(vec3 baseCol, float roughness, float anisotropic, vec3 halfway, vec3 view, vec3 lightDir, vec3 normal,vec3 hl,vec3 wlIn, vec3 wlOut){
    float alphaX;
    float alphaY;
    GetAlphas(alphaX, alphaY, anisotropic, roughness);
    vec3 fm =Fm(baseCol, view, halfway);
    float dm = Dm(roughness, anisotropic, hl, alphaX, alphaY);
    float gm= Gm(roughness, anisotropic, wlIn, wlOut, alphaX, alphaY);
    vec3 num = fm*dm*gm;
    float denom = 4* CosthetaTangent(normal, lightDir);
    return num/denom;
}

float Ro(float n){
    float ro= pow(n - 1.0f,2.0)/pow(n + 1.0f,2.0);
    return ro;
}
// clearcoat
float Fc (vec3 halfway, vec3 view){
    float ro = Ro(N);
    float costhetaTangent= CosthetaTangent(halfway, view);
    float powPart= (1- ro) * pow(1 - costhetaTangent, 5.0f);
    float result = ro + powPart;
    return result;
}
float GetClearcoatGloss(float clearcoatGlossParam){
    float result = ((1.0f - clearcoatGlossParam) * 0.1f) + (clearcoatGlossParam* 0.001);
    return result;
};
float Dc(float clearcoatGloss, vec3 hl){
    float clearcoatGlossPow = pow(clearcoatGloss, 2.0f);
    float hlzPow = pow(hl.z, 2.0f);

    float num = clearcoatGlossPow - 1.0f;
    float denom = (3.14 * log(clearcoatGlossPow)) * (1.0f + ((clearcoatGlossPow - 1)*(hlzPow)));
    return num / denom;
};
 