#include "ModelHandler.h"
#include "FileSystem/FileHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"
#include <string>

namespace VULKAN
{

	ModelHandler* ModelHandler::instance = nullptr;	

	ModelHandler::ModelHandler()
	{
		ModelLoaderHandler::GetInstance();
		baseMaterial.albedoIntensity = 0;
		baseMaterial.normalIntensity = 0;
		baseMaterial.specularIntensity = 0;
		baseMaterial.diffuseColor = glm::vec3(1);
		baseMaterial.textureIndexStart = -1;
		baseMaterial.texturesSizes = 0;
		baseMaterial.diffuseOffset = -1;
        baseMaterial.normalOffset = -1;

	}

	ModelHandler *ModelHandler::GetInstance()
	{
		if (instance==nullptr)
		{
			instance = new ModelHandler;
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
				LoadModel(&modelsReady, path);
			}));
		}
		
	}

	void ModelHandler::LoadModel(std::vector<std::shared_ptr<ModelToLoadState>>* modelsReadyToLoadVec, std::string path)
	{
		//path is already cleaned
		auto modelToLoadState = std::make_shared<ModelToLoadState>();
		modelToLoadState->state = UNLOADED;
		modelToLoadState->model= ModelLoaderHandler::GetInstance()->GetModelVertexAndIndicesTinyObject(path);
		modelToLoadState->state = LOADED;
        std::lock_guard<std::mutex> lock(loadAssetMutex);
        
		modelsReadyToLoadVec->push_back(std::ref(modelToLoadState));
        
        AssetsHandler::GetInstance()->HandleAssetLoad(modelToLoadState->model,path, AssetsHandler::GetInstance()->codeFileExtension);
        AssetsHandler::GetInstance()->AddAssetData(path);       

	}

	void ModelHandler::CreateBLAS(glm::vec3 pos,glm::vec3 rot, glm::vec3 scale,ModelData combinedMesh, RayTracing_RS::TopLevelObj& TLAS)
	{
			VkTransformMatrixKHR matrix = {
			1.0f, 0.0f, 0.0f, pos.x,
			0.0f, 1.0f, 0.0f, pos.y,
			0.0f, 0.0f, -1.0,pos.z};
		RayTracing_RS::BottomLevelObj bottomLevelObj{};
		bottomLevelObj.combinedMesh = combinedMesh;
		bottomLevelObj.pos = pos;
		bottomLevelObj.rot = rot;
		bottomLevelObj.scale = scale;
		bottomLevelObj.matrix = matrix;
		bottomLevelObj.bottomLevelId = static_cast<int>(bottomLevelObjects.at(TLAS.TLASID).size());
		bottomLevelObj.UpdateMatrix();
		bottomLevelObjects.at(TLAS.TLASID).push_back(bottomLevelObj);
		
	}

	void ModelHandler::AddTLAS(RayTracing_RS::TopLevelObj& topLevelObj)
	{
		std::vector<RayTracing_RS::BottomLevelObj> BLASes;
		bottomLevelObjects.try_emplace(TLASesCount,BLASes);
		topLevelObj.topLevelInstanceCount = 1;
		topLevelObj.TLASID = TLASesCount;
		TLASesCount++;
	}

	std::vector<RayTracing_RS::BottomLevelObj>& ModelHandler::GetBLASesFromTLAS(RayTracing_RS::TopLevelObj TLAS)
	{
		return bottomLevelObjects.at(TLAS.TLASID);
	}

	RayTracing_RS::BottomLevelObj& ModelHandler::GetBLASFromTLAS(RayTracing_RS::TopLevelObj TLAS, int index)
	{
		return bottomLevelObjects.at(TLAS.TLASID)[index];
	}

    void ModelHandler::CreateMaterialTextures(VulkanSwapChain& swapChain) {

        for (int i = 0; i < allMaterialsOnApp.size(); ++i)
        {
            allMaterialsOnApp[i].get()->CreateTextures(swapChain,allTexturesOffset);
        }
    }
}
