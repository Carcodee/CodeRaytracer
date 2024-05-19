#include "ModelHandler.h"


namespace VULKAN
{

	ModelHandler* ModelHandler::instance = nullptr;	

	ModelHandler::ModelHandler()
	{
	}

	ModelHandler *ModelHandler::GetInstance()
	{
		if (instance==nullptr)
		{
			instance = new ModelHandler;
		}
		return instance;
	}

	void ModelHandler::CreateBLAS(glm::vec3 pos,glm::vec3 rot, glm::vec3 scale,std::vector<Vertex>vertices,std::vector<uint32_t> indices, RayTracing_RS::TopLevelObj TLAS)
	{
		RayTracing_RS::BottomLevelObj bottomLevelObj{};
		bottomLevelObj.vertices = vertices;
		bottomLevelObj.indices = indices;
		bottomLevelObj.pos = pos;
		bottomLevelObj.rot = rot;
		bottomLevelObj.scale = scale;
		bottomLevelObj.bottomLevelId = bottomLevelObjects.at(TLAS.TLASID).size();
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
}
