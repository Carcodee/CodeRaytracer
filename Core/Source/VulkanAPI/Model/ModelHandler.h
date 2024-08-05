#pragma once
#include <future>
#include <mutex>

#include "VulkanAPI/RenderSystems/RayTracing_RS.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/Utility/Utility.h"

namespace VULKAN
{
	
	class ModelHandler
	{
	protected:

		static ModelHandler* instance;
		ModelHandler();

	public:
		enum STATE
		{
			UNLOADED,
			LOADED,
			DISPACHED,
			INVALID

		};
		struct ModelToLoadState
		{
			std::shared_ptr<ModelData> model;
			STATE state = UNLOADED;

		};


		ModelHandler(ModelHandler& other) = delete;
		void operator=(const ModelHandler&) = delete;

		static ModelHandler *GetInstance();

		void AddModelToQuery(std::string path);
        void AddIdToQuery(int id);
		void LoadModel(std::vector<std::shared_ptr<ModelToLoadState>>* modelsReadyToLoadVec, std::string path);
        void LoadModelFromDisc(std::vector<std::shared_ptr<ModelToLoadState>>* modelsReadyToLoadVec, int id);
        void LoadAllModelsFromDisc();
		void LoadAllModels();
        void CreateMaterialTextures(VulkanSwapChain& swapChain);
        void ReCalculateMaterialOffsets();
        void AddMaterial(Material& material);
        void AddSphere();
        void CreateMaterial(std::string path);
        void CreateInstance(Material& material);

		void CreateBLAS(glm::vec3 pos,glm::vec3 rot, glm::vec3 scale,ModelData combinedMesh, TopLevelObj& TLAS);
		void AddTLAS(TopLevelObj& bottomLevelObj);
        
        
		std::vector<BottomLevelObj>& GetBLASesFromTLAS(TopLevelObj TLAS);
		BottomLevelObj& GetBLASFromTLAS(TopLevelObj TLAS, int index);
		std::vector<std::string> queryModelPathsToHandle;
        std::vector<int> queryModelIdsToHandle;
		std::vector<std::shared_ptr<ModelToLoadState>> modelsReady;
		std::map<int,std::vector<BottomLevelObj>> bottomLevelObjects;
		std::vector<std::future<void>> futures;
		std::mutex loadAssetMutex;
		bool Loading = false;
        bool updateMeshData = false;
        bool updateMaterialData = false;
        bool updateBottomLevelObj = false;
		int TLASesCount = 0;
        int allTexturesOffset =0;
        int currentMaterialsOffset= 0;
		MaterialUniformData baseMaterialUniformData{};
        Material materialBase{};
        std::map<int,std::shared_ptr<Material>> allMaterialsOnApp;
        std::map<int,std::shared_ptr<ModelData>> allModelsOnApp;
        std::map<int,std::shared_ptr<VKTexture>>allTexturesOnApp;
        std::vector<Sphere> allSpheresOnApp;

	};

}
