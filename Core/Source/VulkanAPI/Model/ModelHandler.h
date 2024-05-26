#pragma once
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

		ModelHandler(ModelHandler& other) = delete;
		void operator=(const ModelHandler&) = delete;

		static ModelHandler *GetInstance();

		std::map<int,std::vector<RayTracing_RS::BottomLevelObj>> bottomLevelObjects;

		void CreateBLAS(glm::vec3 pos,glm::vec3 rot, glm::vec3 scale,std::vector<Vertex>vertices,std::vector<uint32_t> indices, RayTracing_RS::TopLevelObj& TLAS);
		void AddTLAS(RayTracing_RS::TopLevelObj& bottomLevelObj);
		std::vector<RayTracing_RS::BottomLevelObj>& GetBLASesFromTLAS(RayTracing_RS::TopLevelObj TLAS);
		RayTracing_RS::BottomLevelObj& GetBLASFromTLAS(RayTracing_RS::TopLevelObj TLAS, int index);
		int TLASesCount = 0;

	};

}
