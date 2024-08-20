
// w out is view dir, could change later
float CosthetaTangent(vec3 v1, vec3 v2){
    return max(dot(v1, v2), 0.001);
}

float FD90(float roughness, vec3 halfway, vec3 normal){
    float costhetaTangent = CosthetaTangent(halfway, normal);
    float secondSumPart = (2*roughness)*pow(costhetaTangent, 2.0);
    return 0.5 + secondSumPart; 
};
float FSS90(float roughness, vec3 halfway, vec3 view){
    float costhetaTangent = CosthetaTangent(halfway, view);
    float result = roughness * pow(costhetaTangent, 2.0f);
    return result;
}
float FSSW(vec3 halfway, vec3 view, vec3 w, vec3 normal, float roughness){
    float fss90= FSS90(roughness, halfway, view);
    float costhetaTangent = CosthetaTangent(w, normal);
    float result = (1 + ((fss90 - 1)*(pow(1 - costhetaTangent, 5))));
    return result;
}

vec3 DisneyFS(vec3 baseCol, vec3 normal, vec3 halfway, vec3 view, vec3 lightDir, float rougness){
    
    vec3 colPart = (baseCol* 1.25)/3.14;
    float FSSWin = FSSW(halfway, view, lightDir, normal, rougness);
    float FSSWOut = FSSW(halfway, view, view, normal, rougness);
    float costhetaLight = CosthetaTangent(lightDir, normal);
    float costhetaView = CosthetaTangent(view, normal);
    
    float division = (1/(costhetaLight + costhetaView)) - 0.5f;
    
    vec3 fSubsurface = colPart* ((FSSWin* FSSWOut*division) + 0.5f) * costhetaView;
    return fSubsurface;
}
vec3 DisneyFD(vec3 baseCol, vec3 normal, vec3 halfway, vec3 view, vec3 lightDir, float rougness){
    vec3 lambert = baseCol/3.14f;
    float costhetaLight = CosthetaTangent(lightDir, normal);
    float costhetaView = CosthetaTangent(view, normal);
    float FD90Val = FD90(rougness, halfway, normal);
    float fdPart = FD90Val - 1;
    float viewPart = pow(1 - costhetaView, 5);
    float lightPart = pow(1 - costhetaLight, 5);
    vec3 fd = lambert*(1 + (fdPart * lightPart))*(1 + (fdPart * viewPart));
    
    return fd;
};

vec3 DisneyDiffuse(vec3 baseCol, vec3 normal, vec3 halfway, vec3 view, vec3 lightDir, float rougness, float subsurface){
    vec3 fd = DisneyFD(baseCol, normal, halfway, view, lightDir, rougness);
    vec3 fs = DisneyFS(baseCol, normal, halfway, view, lightDir, rougness);
    
    vec3 result = ((1 - subsurface)* fd) + (subsurface*fs);
    return result;
}
