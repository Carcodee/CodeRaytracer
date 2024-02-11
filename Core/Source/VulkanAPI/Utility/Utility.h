#pragma once


#ifdef false
#define LOG() std::cout
#else
#define LOG() while(false) std::cout
#endif




#include "VulkanAPI/Model/MyModel.h"


namespace VULKAN{


		//std::vector<MyModel::Vertex> GetVertexPosForRecursiveTriangles(std::vector<MyModel::Vertex> vertices,int maxDeepness);

		//std::vector<MyModel::triangle> FindTriangles(std::vector<MyModel::triangle> myTriangle, int deep, int& currentDeppness);
		//std::vector<MyModel::triangle> FindTriMidPoint(MyModel::triangle myTriangle);
		//std::vector<MyModel::Vertex> GetAllVertexFlatten(std::vector<MyModel::triangle> triangles);
}




