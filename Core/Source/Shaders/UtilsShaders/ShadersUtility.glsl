

#include "../UtilsShaders/Random.glsl"
#include "../UtilsShaders/DisneyShadingModel.glsl"
#define DIFFUSE_TEX 0 
#define ALPHA_TEX 1 
#define SPECULAR_TEX 2 
#define BUMP_TEX 3 
#define AMBIENT_TEX 4 

struct RayPayload{
	vec3 color;
	vec3 colorLit;
	vec3 emissionColor;
	float distance;
	vec3 normal;
	vec3 tangent;
	vec3 origin;
	vec3 direction;
	vec3 sampleDir;
	float roughness;
	float reflectivity;
	float hitT;
    uvec2 frameSeed;	
	bool shadow;
	bool isMiss;
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
	float specularIntensity;
	float roughnessIntensity;
	vec4 diffuseColor;
	float reflectivityIntensity;
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
float PDF_GGX(vec3 normal, vec3 halfWay, vec3 view, float roughness) {
	float D = D_GGX(roughness, normal, halfWay);
	float cosThetaH = max(dot(normal, halfWay), 0.0001);
	float dotHV = max(dot(halfWay, view), 0.0001);
	return (D * cosThetaH) / (4.0 * dotHV);
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
vec3 GetBRDF(vec3 normal, vec3 wo, vec3 wi,vec3 wh,vec3 col, vec3 FO, float metallic,  float roughness){

	float D = D_GGX(roughness, normal, wh);
	float G = G(roughness, normal, wo, wi);
	vec3 F = FresnelShilck(wh, wo, FO);
	vec3 cookTorrence = CookTorrance(normal, wo, wi, D, G, F);
	vec3 lambert= LambertDiffuse(col);
	vec3 diffuse = DisneyDiffuse(col, normal, wh, wo, wi, roughness, 0.9f);
	vec3 ks = F;
	vec3 kd = (vec3(1.0) - ks) * (1 - metallic);
	vec3 BRDF =  (kd * diffuse) + cookTorrence;
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
float oldRand(float uvX, float uvY) {
	return fract(sin(uvX * 12.9898 + uvY * 78.233) * 43758.5453123);
}

vec3 randomCosineWeightedDirection(vec3 normal, vec2 seed, uint frame) {

	float r1 =oldRand(seed.x, seed.y);
	float r2 =oldRand(seed.x, seed.y);
	// Convert to spherical coordinates
	float theta = acos(sqrt(1.0 - r1));
	float phi = 2.0 * PI * r2;

	// Convert spherical coordinates to Cartesian coordinates
	float x = sin(theta) * cos(phi);
	float y = sin(theta) * sin(phi);
	float z = cos(theta);

	vec3 tangent= cross(vec3(x,y,z), normal);
	vec3 bitangent = cross(normal, tangent);
	// Convert the sample to world coordinates
	vec3 sampleDir = x * tangent + y * bitangent + z * normal;
	return normalize(sampleDir);
}

//pdf for brdf in my case
float CosinePdfHemisphere(float cosTheta)
{
	return cosTheta / PI;
}

float MaxComponent(vec3 v)
{
	return max(v.x, max(v.y, v.z));
}

void CreateOrthonormalBasis(in vec3 N, out vec3 T, out vec3 B)
{
	if (abs(N.z) > 0.999)
	{
		T = vec3(1.0, 0.0, 0.0);
	}
	else
	{
		T = normalize(cross(vec3(0.0, 0.0, 1.0), N));
	}
	B = cross(N, T);
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
