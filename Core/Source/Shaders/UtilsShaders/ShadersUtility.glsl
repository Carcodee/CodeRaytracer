

#include "../UtilsShaders/Random.glsl"
#include "../UtilsShaders/Functions.glsl"
#include "../UtilsShaders/Shading.glsl"

#ifndef Utils 
#define Utils 

struct RayPayload{
	vec3 color;
	vec3 colorLit;
	vec3 emissionColor;
	float pdf;
	float distance;
	vec3 normal;
	vec3 origin;
	vec3 direction;
	vec3 sampleDir;
	float hitT;
    uvec2 frameSeed;	
	uint frame;
	bool shadow;
	bool isMiss;
	bool stop;
};
struct MeshData {
	int materialIndexOnShape;
	int geometryIndexStartOffset;
	int indexOffset;
};

struct MaterialConfigurations{
	bool alphaAsDiffuse;
	bool useAlphaChannel;
	bool useDiffuseColorAlpha;
	bool useDisneyBSDF;
	bool thin;
};

struct Vertex {
	vec3 position;
	vec3 col;
	vec3 normal;
	vec3 tangent;
	vec2 texCoords;
};

struct Indices {
	uint index;
};

struct MaterialFindInfo{
	bool hasDiffuse;
	bool hasNormals;
};

struct Surface{
	vec3 normal;
	vec3 tangent;
	vec3 pos;
	vec3 uv;
};

struct Material{
	vec3 diffuse;
	vec3 baseReflectiveness;
	vec3 directLightDir;
	float emissiveMesh;
	float roughness;
	float reflectivity;
};
struct MaterialData {
	float albedoIntensity;
	float normalIntensity;
	float roughnessIntensity;
	vec4 diffuseColor;
	vec3 transColor;
	vec3 baseReflection;
	float metallicIntensity;
	float emissionIntensity;
	int roughnessOffset;
	int metallicOffset;
	int emissionOffset;
	int metallicRouhgnessOffset;
	float alphaCutoff;
	int diffuseOffset;
	int normalOffset;
	uint configurations;
	
	// disney bsdf
	float anisotropic;
	float subSurface;
	float clearcoat;
	float clearcoatGloss;
	float specularTransmission;
	float diffTransmission;
	float scatterDistance;
	float ior;
	float relativeIOR;
	float flatness;
	float sheen;
	float specular;
    float specularTint;
	float sheenTint;
};
MaterialFindInfo GetMatInfo(vec4 diffuse, vec4 normal){
	
	MaterialFindInfo materialFindInfo;
	materialFindInfo.hasDiffuse= true;
	materialFindInfo.hasNormals= true;
	if(diffuse==vec4(1)){
		materialFindInfo.hasDiffuse = false;
	}
	if(normal==vec4(1)){
		materialFindInfo.hasNormals = false;
	}
	return materialFindInfo;
}

void GetMatConfigs(uint configs, out MaterialConfigurations materialConfigurations){
	materialConfigurations.alphaAsDiffuse = (configs & 1) != 0;
	materialConfigurations.useAlphaChannel = (configs & (1 << 1)) != 0;
	materialConfigurations.useDiffuseColorAlpha = (configs & (1 << 2)) != 0;
	materialConfigurations.useDisneyBSDF = (configs & (1 << 3)) != 0;
	materialConfigurations.thin = (configs & (1 << 4)) != 0;
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

bool IsBlack(vec3 color)
{
	return dot(color, color) < EPSILON;
}

float PDF_GGX(vec3 normal, vec3 halfWay, vec3 view, float roughness) {
	float D = D_GGX(roughness, normal, halfWay);
	float cosThetaH = max(dot(normal, halfWay), 0.0001);
	float dotHV = max(dot(halfWay, view), 0.0001);
	return (D * cosThetaH) / (4.0 * dotHV);
}

float G1( float rougness, vec3 xVector, vec3 normal){
	
	float k = rougness/2;
	float dot1= max(dot(normal, xVector), 0.0001);
	float denom= (dot1* (1-k)) +k;
	return dot1/denom;
}

float G(float alpha, vec3 N, vec3 V, vec3 L){
	return G1(alpha,  N, V) * G1(alpha,  N, L);
}


vec3 GetBRDF(vec3 normal, vec3 wo, vec3 wi,vec3 wh,vec3 col, vec3 FO, float metallic,  float roughness){

	float D = D_GGX(roughness, normal, wh);
	float G = G(roughness, normal, wo, wi);
	vec3 F = FresnelShilck(wh, wo, FO);
	vec3 cookTorrence = CookTorrance(normal, wo, wi, D, G, F);
	vec3 lambert= LambertDiffuse(col);
	vec3 ks = F;
	vec3 kd = (vec3(1.0) - ks) * (1 - metallic);
	vec3 BRDF =  (kd * lambert) + cookTorrence;
	return BRDF;
}

vec3 GetPBR (vec3 col,vec3 lightCol, float emissiveMesh, float roughness,float metallic,  vec3 baseReflectivity, vec3 normal, vec3 view,vec3 light, vec3 halfWay, float cosThetaTangent){

	vec3 BRDF = GetBRDF(normal, view, light, halfWay, col, baseReflectivity, metallic, roughness);
	vec3 outgoingLight = emissiveMesh + BRDF * lightCol * cosThetaTangent;
	return outgoingLight;
}

vec3 GetPBRLit (vec3 col,vec3 lightCol, float emissiveMesh, float roughness,float metallic,  vec3 baseReflectivity, vec3 normal, vec3 view,vec3 light, vec3 halfWay){
	vec3 BRDF = GetBRDF(normal, view, light, halfWay, col, baseReflectivity, metallic, roughness);
	
	return BRDF;
}

float MaxComponent(vec3 v)
{
	return max(v.x, max(v.y, v.z));
}

void CreateOrthonormalBasisWithTangent(in vec3 N, in vec3 T, out vec3 B)
{
	B = cross(N, T);
}
vec3 TangentToWorld(vec3 sampleVec, vec3 normal)
{
	vec3 T, B;
	CreateOrthonormalBasis(normal, T, B);

	// TBN matrix
	mat3 TBN = mat3(T, B, normal);

	// Transform the sample vector
	return TBN * sampleVec;
}
vec3 TangentToWorldWithTangent(vec3 sampleVec, vec3 normal, vec3 tangent)
{
	vec3 B;
	CreateOrthonormalBasisWithTangent(normal, tangent, B);

	// TBN matrix
	mat3 TBN = mat3(tangent, B, normal);

	// Transform the sample vector
	return TBN * sampleVec;
}
vec3 GetReflection(vec3 reflectedDir, vec3 randomDir, float rougness){
	return normalize(mix(reflectedDir, randomDir, rougness));
}

#endif 