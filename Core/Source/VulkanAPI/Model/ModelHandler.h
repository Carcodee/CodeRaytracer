#pragma once
#include <future>
#include <mutex>

#include "VulkanAPI/RenderSystems/RayTracing_RS.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"

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
			ModelData model;
			STATE state = UNLOADED;

		};


		ModelHandler(ModelHandler& other) = delete;
		void operator=(const ModelHandler&) = delete;

		static ModelHandler *GetInstance();

		void AddModelToQuery(std::string path);
		void LoadModel(std::vector<std::shared_ptr<ModelToLoadState>>* modelsReadyToLoadVec, std::string path);
		void LoadAllModels();
        void CreateMaterialTextures(VulkanSwapChain& swapChain);

		void CreateBLAS(glm::vec3 pos,glm::vec3 rot, glm::vec3 scale,ModelData combinedMesh, RayTracing_RS::TopLevelObj& TLAS);
		void AddTLAS(RayTracing_RS::TopLevelObj& bottomLevelObj);
        Material& GetMaterialFromPath(std::string path);
        
        
		std::vector<RayTracing_RS::BottomLevelObj>& GetBLASesFromTLAS(RayTracing_RS::TopLevelObj TLAS);
		RayTracing_RS::BottomLevelObj& GetBLASFromTLAS(RayTracing_RS::TopLevelObj TLAS, int index);
		std::vector<std::string> queryModelPathsToHandle;
		std::vector<std::shared_ptr<ModelToLoadState>> modelsReady;
		std::map<int,std::vector<RayTracing_RS::BottomLevelObj>> bottomLevelObjects;
		std::vector<std::future<void>> futures;
		std::mutex loadAssetMutex;
		bool Loading = false;
		int TLASesCount = 0;
        int allTexturesOffset =0;
		MaterialUniformData baseMaterial{};
        std::vector<std::shared_ptr<Material>> allMaterialsOnApp;
        std::unordered_map<std::string,std::shared_ptr<ModelData>> allModelsOnApp;

	};

}
