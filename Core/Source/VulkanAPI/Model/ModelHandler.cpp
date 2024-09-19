#include "ModelHandler.h"
#include "FileSystem/FileHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"
#include "VulkanAPI/RenderSystems/ImguiRenderSystem.h"
#include <string>

namespace VULKAN
{

	ModelHandler* ModelHandler::instance = nullptr;	

	ModelHandler::ModelHandler(VulkanRenderer* renderer)
	{
		ModelLoaderHandler::GetInstance();
		baseMaterialUniformData.albedoIntensity = 1.0f;
		baseMaterialUniformData.normalIntensity = 0;
		baseMaterialUniformData.diffuseColor = glm::vec4(1.0f);
		baseMaterialUniformData.metallicRoughnessOffset = -1;
		baseMaterialUniformData.alphaCutoff = 0;
		baseMaterialUniformData.diffuseOffset = -1;
        baseMaterialUniformData.normalOffset = -1;
        materialBase.materialUniform = baseMaterialUniformData;
        materialBase.id = 0;
        materialBase.name= "standard_mat";
        this->renderer = renderer;
        allSpheresOnApp.reserve(MAX_SPHERES_ON_APP);
        

	}

	ModelHandler *ModelHandler::GetInstance(VulkanRenderer* renderer)
	{
		if (instance==nullptr)
		{
            assert(renderer != nullptr && "A renderer must be passed on creation");
			instance = new ModelHandler(renderer);
		}
		return instance;
	}

	void ModelHandler::AddModelToQuery(std::string path)
	{

		std::string realPath = HELPERS::FileHandler::GetInstance()->HandleModelFilePath(path);

		if (realPath=="")
		{
			std::cout << "Path: " << path << " is not valid";
			return;
		}

		queryModelPathsToHandle.push_back(realPath);

	}

	void ModelHandler::LoadAllModels()
	{
		if (Loading)return;
		for (auto path : queryModelPathsToHandle)
		{
			Loading = true;
			futures.push_back(std::async(std::launch::async,[this, path]()
			{
                MODEL_FORMAT format;
                
                if(HELPERS::FileHandler::GetInstance()->GetPathExtension(path) == ".obj"){
                    format = OBJ;
                } else if(HELPERS::FileHandler::GetInstance()->GetPathExtension(path) == ".gltf"){
                    format = GLTF;
                }
				LoadModel(&modelsReady, path, format);
			}));
		}
		
	}

	void ModelHandler::LoadModel(std::vector<std::shared_ptr<ModelToLoadState>>* modelsReadyToLoadVec, std::string path, MODEL_FORMAT modelFormat)
	{
		//path is already cleaned
		auto modelToLoadState = std::make_shared<ModelToLoadState>();
		modelToLoadState->state = UNLOADED;
        switch (modelFormat) {
            case OBJ:
                modelToLoadState->model= std::make_shared<ModelData>(ModelLoaderHandler::GetInstance()->GetModelVertexAndIndicesTinyObject(path));
                break;
            case GLTF:
                tinygltf::Model modelGLTF;
                modelToLoadState->model = std::make_shared<ModelData>(); 
                ModelLoaderHandler::GetInstance()->LoadGLTFModel(path,modelGLTF, *modelToLoadState->model);
                break;
        }
        modelToLoadState->model->pathToAssetReference = path;
		modelToLoadState->state = LOADED;
        std::lock_guard<std::mutex> lock(loadAssetMutex);
		modelsReadyToLoadVec->push_back(std::ref(modelToLoadState));
        //runtime
        modelToLoadState->model->id =allModelsOnApp.size();
        assert(!allModelsOnApp.contains(modelToLoadState->model->id) &&"Two models contains the same id");
        allModelsOnApp.try_emplace(modelToLoadState->model->id,modelToLoadState->model);
        std::string metaFilePath=AssetsHandler::GetInstance()->HandleAssetLoad<ModelData>(*modelToLoadState->model,path, AssetsHandler::GetInstance()->codeModelFileExtension, modelToLoadState->model->id);
	}

    void ModelHandler::LoadModelFromDisc(std::vector<std::shared_ptr<ModelToLoadState>> *modelsReadyToLoadVec, int id) {
        auto modelToLoadState = std::make_shared<ModelToLoadState>();
        std::string path = allModelsOnApp.at(id)->pathToAssetReference;
        modelToLoadState->state = UNLOADED;
        modelToLoadState->model= std::make_shared<ModelData>();
        modelToLoadState->model->modelFormat=allModelsOnApp.at(id)->modelFormat;
        tinygltf::Model gltfModel;
        tinyobj::ObjReader reader;
        switch (modelToLoadState->model->modelFormat) {
            case OBJ:
                ModelLoaderHandler::GetInstance()->FindReader(reader,path);
                ModelLoaderHandler::GetInstance()->GetModelFromReader(reader, *modelToLoadState->model);
                break;
            case GLTF:
                ModelLoaderHandler::GetInstance()->GetGLTFModel(gltfModel,path);
                ModelLoaderHandler::GetInstance()->LoadGLTFFromModel(gltfModel, *modelToLoadState->model);
                break;
        }
        
        modelToLoadState->model->materialOffset =allModelsOnApp.at(id)->materialOffset;
        modelToLoadState->model->materialIds = allModelsOnApp.at(id)->materialIds;
        modelToLoadState->model->id = id;
        modelToLoadState->model->pathToAssetReference = path;
        modelToLoadState->state = LOADED;
        allModelsOnApp.at(id)=modelToLoadState->model;
        std::lock_guard<std::mutex> lock(loadAssetMutex);
        modelsReadyToLoadVec->push_back(std::ref(modelToLoadState));
        //runtime
    }

    void ModelHandler::LoadAllModelsFromDisc() {
        if (Loading)return;
        for (auto id : queryModelIdsToHandle)
        {
            Loading = true;
            futures.push_back(std::async(std::launch::async,[this, id]()
            {
                LoadModelFromDisc(&modelsReady,id);
            }));
        }
    }
	void ModelHandler::CreateBLAS(glm::vec3 pos,glm::vec3 rot, glm::vec3 scale,ModelData& combinedMesh, TopLevelObj& TLAS)
	{
        VkTransformMatrixKHR matrix = {
                1.0f, 0.0f, 0.0f, pos.x,
                0.0f, 1.0f, 0.0f, pos.y,
                0.0f, 0.0f, -1.0,pos.z};

		BottomLevelObj bottomLevelObj{};
		bottomLevelObj.combinedMesh = combinedMesh;
		bottomLevelObj.pos = pos;
		bottomLevelObj.rot = rot;
		bottomLevelObj.scale = scale;
		bottomLevelObj.matrix = matrix;
		bottomLevelObj.bottomLevelId = static_cast<int>(bottomLevelObjects.at(TLAS.TLASID).size());
		bottomLevelObj.UpdateMatrix();
        
		bottomLevelObjects.at(TLAS.TLASID).push_back(bottomLevelObj);
		
	}

	void ModelHandler::AddTLAS(TopLevelObj& topLevelObj)
	{
		std::vector<BottomLevelObj> BLASes;
		bottomLevelObjects.try_emplace(TLASesCount,BLASes);
		topLevelObj.topLevelInstanceCount = 1;
		topLevelObj.TLASID = TLASesCount;
		TLASesCount++;
	}

	std::vector<BottomLevelObj>& ModelHandler::GetBLASesFromTLAS(TopLevelObj TLAS)
	{
        return bottomLevelObjects.at(TLAS.TLASID);
	}

	BottomLevelObj& ModelHandler::GetBLASFromTLAS(TopLevelObj TLAS, int index)
	{
		return bottomLevelObjects.at(TLAS.TLASID)[index];
	}

    void ModelHandler::CreateMaterialTextures(VulkanSwapChain& swapChain) {

        int counter = 0;
        for (auto& mat: allMaterialsOnApp)
        {
            mat.second.get()->CreateTextures(swapChain,allTexturesOffset);
            std::string path= mat.second->targetPath;
            AssetsHandler::GetInstance()->HandleAssetLoad<Material>(*mat.second.get() ,path , AssetsHandler::GetInstance()->matFileExtension, mat.second->id);
            counter++;
        }
    }


    void ModelHandler::AddIdToQuery(int id) {
        if(allModelsOnApp.contains(id)){
            queryModelIdsToHandle.push_back(id);
        } else{
            std::cout<< "ID Does not exist: "<<std::to_string(id)<<"\n";
        }
    }

    void ModelHandler::ReCalculateMaterialOffsets() {
        currentMaterialsOffset=0;
        for (int i = 0; i < allMaterialsOnApp.size(); ++i) {
            currentMaterialsOffset ++;
        }
        
    }

    void ModelHandler::AddMaterial(Material &material) {
        material.id= currentMaterialsOffset;
        material.name = "Material_"+ std::to_string(material.id);
        allMaterialsOnApp.try_emplace(currentMaterialsOffset, std::make_shared<Material>(material));
        
    }

    void ModelHandler::CreateMaterial(std::string path) {
        Material material{};
        material.textureReferencePath="";
        bool generated = false;
        AddMaterial(material);
        ReCalculateMaterialOffsets();
        //this data is filled in that methods, weird, I know
        material.targetPath = path +"\\"+ material.name + ".MATCODE";
        AssetsHandler::GetInstance()->HandleAssetLoad(material,material.targetPath, AssetsHandler::GetInstance()->matFileExtension, material.id);
//        updateBottomLevelObj = true;
    }

    void ModelHandler::CreateInstance(Material &material) {

    }

    void ModelHandler::AddSphere() {
        Sphere sphere{};
        sphere.sphereUniform.pos = glm::vec3 (0.0f);
        sphere.sphereUniform.radius = 1;
        sphere.sphereUniform.matId= 0;
        sphere.sphereUniform.instanceOffset = 0;
        sphere.id =allSpheresOnApp.size();
        
        allSpheresOnApp.push_back(sphere);
    }

    void ModelHandler::AddTexture(std::string path) {

        assert(AssetsHandler::GetInstance()->IsValidImageFormat(HELPERS::FileHandler::GetInstance()->GetPathExtension(path))
        &&"When creating an image it must be a valid format");
        VKTexture* texture = new VKTexture(path.c_str(), renderer->GetSwapchain(), true);
        std::cout<<"Image added with id: " << texture->id<<"\n";
    }

}
