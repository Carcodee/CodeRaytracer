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
}
