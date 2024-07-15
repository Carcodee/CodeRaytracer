


#define DIFFUSE_TEX 0 
#define ALPHA_TEX 1 
#define SPECULAR_TEX 2 
#define BUMP_TEX 3 
#define AMBIENT_TEX 4 

#define PI 3.1415

struct MaterialFindInfo{
	bool hasDiffuse;
	bool hasNormals;
};

MaterialFindInfo GetMatInfo(vec4 diffuse, vec4 normal){
	
	MaterialFindInfo materialFindInfo;
	materialFindInfo.hasDiffuse= true;
	materialFindInfo.hasNormals= true;
	if(diffuse==vec4(-1)){
		materialFindInfo.hasDiffuse = false;
	}
	if(diffuse==vec4(-1)){
		materialFindInfo.hasNormals = false;
	}
	return materialFindInfo;
}

vec3 LambertDiffuse(vec3 col){
	return col/PI;
}

vec3 CookTorrance(vec3 normal, vec3 view,vec3 light, float D, float G, vec3 F){
	vec3 DGF = D*G*F;
	float dot1 = max(dot(view, normal), 0.0001);
	float dot2 = max(dot(light, normal), 0.0001);
	float dotProducts= 4 * dot1 * dot2;
	return DGF/dotProducts;
}

float D_GGX(float roughness, vec3 normal, vec3 halfway){
	float dot = max(dot(normal,halfway), 0.0001);
	dot = pow(dot,2.0);
	float roughnessPart = pow(roughness,2.0)-1;
	float denom = PI* pow(((dot * roughnessPart)+1), 2.0);
	return pow(roughness,2.0)/denom;
}

//xVector could be view or light vector
float G1( float rougness, vec3 xVector, vec3 normal){
	
	float k = rougness/2;
	float dot1= max(dot(normal, xVector), 0.0001);
	float denom= (dot1* (1-k)) +k;
	return dot1/denom;
}

float G(float alpha, vec3 N, vec3 V, vec3 L){
	return G1(alpha,  N, V) * G1(alpha,  N, L);
}

vec3 FresnelShilck(vec3 halfway, vec3 view, vec3 FO){
	float powPart= 1- max(dot(view, halfway),0.0001);
	powPart =pow(powPart,5);
	vec3 vecPow = powPart * (vec3(1.0)-FO);
	return FO + vecPow;
}

vec3 GetPBR (vec3 col,vec3 lightCol, float emissiveMesh, float roughness,float metallic,  vec3 specular, vec3 normal, vec3 view,vec3 light, vec3 halfWay){
	float D = D_GGX(roughness, normal, halfWay);
	float G = G(roughness, normal, view, light);
	vec3 F = FresnelShilck(halfWay, view, specular);
	vec3 cookTorrence = CookTorrance(normal, view, light, D, G, F);
	vec3 lambert= LambertDiffuse(col);
	vec3 ks = F;
	vec3 kd = (vec3(1.0) - ks) * (1 - metallic);
	vec3 BRDF = lambert * kd + cookTorrence;
	vec3 outgoingLight = emissiveMesh + BRDF * lightCol * max(dot(light,normal), 0.0001);
	return outgoingLight;
}




