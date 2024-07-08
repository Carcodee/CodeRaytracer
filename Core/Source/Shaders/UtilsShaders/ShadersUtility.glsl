


#define DIFFUSE_TEX 0 
#define ALPHA_TEX 1 
#define SPECULAR_TEX 2 
#define BUMP_TEX 3 
#define AMBIENT_TEX 4 
	
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
