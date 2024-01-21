#include "Utility.h"

namespace VULKAN {
	std::vector<MyModel::Vertex> GetVertexPosForRecursiveTriangles(std::vector<MyModel::Vertex> vertices, int maxDeepness)
	{

		std::vector<MyModel::triangle> triangles;

		MyModel::triangle tri{};
		tri.vertices = vertices;
		triangles.push_back(tri);
		int deepness=0;

		triangles = FindTriangles(triangles, 5, deepness);

		return GetAllVertexFlatten(triangles);
	}
	std::vector<MyModel::triangle> FindTriangles(std::vector<MyModel::triangle> myTriangle, int deep, int& currentDeppness)
	{
		std::vector<MyModel::triangle> triangles;
		if (currentDeppness >= deep)
		{

			std::cout << "Deepnesss: " << currentDeppness << "\n";

			return myTriangle;
		}
		else
		{
			int tris = 0;
			for (size_t i = 0; i < myTriangle.size(); i++)
			{
				std::vector<MyModel::triangle> newTris = FindTriMidPoint(myTriangle[i]);

				for (size_t j = 0; j < newTris.size(); j++)
				{
					triangles.push_back(newTris[j]);

					tris++;
				}
			}
			currentDeppness++;
			return FindTriangles(triangles, deep ,currentDeppness);
		}
	}

	std::vector<MyModel::triangle> FindTriMidPoint(MyModel::triangle myTriangle)
	{
		std::vector<MyModel::triangle> triangles;

		MyModel::Vertex vertex3{};
		MyModel::Vertex vertex4{};
		MyModel::Vertex vertex5{};

		vertex3.position = ((myTriangle.vertices[0].position + myTriangle.vertices[1].position) / 2.0f);
		vertex4.position = ((myTriangle.vertices[1].position + myTriangle.vertices[2].position) / 2.0f);
		vertex5.position = ((myTriangle.vertices[2].position + myTriangle.vertices[0].position) / 2.0f);

		vertex3.color = glm::vec3(vertex3.position, 1.0f);
		vertex4.color = glm::vec3(vertex4.position.x, 1.0f,0.0f);
		vertex5.color = glm::vec3(1.0f, vertex5.position);

		// Triangle at the corner 0
		MyModel::triangle tri0{};
		tri0.vertices.push_back(myTriangle.vertices[0]);
		tri0.vertices.push_back(vertex3);
		tri0.vertices.push_back(vertex5);
		triangles.push_back(tri0);

		// Triangle at the corner 1
		MyModel::triangle tri1{};
		tri1.vertices.push_back(vertex3);
		tri1.vertices.push_back(myTriangle.vertices[1]);
		tri1.vertices.push_back(vertex4);
		triangles.push_back(tri1);

		// Triangle at the corner 2
		MyModel::triangle tri2{};
		tri2.vertices.push_back(vertex5);
		tri2.vertices.push_back(vertex4);
		tri2.vertices.push_back(myTriangle.vertices[2]);
		triangles.push_back(tri2);



		return triangles;
	}

	std::vector<MyModel::Vertex> GetAllVertexFlatten(std::vector<MyModel::triangle> triangles)
	{
		std::vector<MyModel::Vertex> vertices;
		for (size_t i = 0; i < triangles.size(); i++)
		{


			LOG() << "Triangle: " << i;
			LOG() << "\n";


			for (size_t j = 0; j < 3; j++)
			{
				vertices.push_back(triangles[i].vertices[j]);

			}

		}

		return vertices;
	}

}