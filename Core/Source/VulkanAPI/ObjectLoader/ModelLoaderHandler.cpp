#include "ModelLoaderHandler.h"
#include <filesystem>
#include <fstream>
#include <unordered_set>

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE

namespace VULKAN {

	ModelLoaderHandler::ModelLoaderHandler(MyVulkanDevice& device) : myDevice{device}
	{

	}
	VKBufferHandler* ModelLoaderHandler::LoadModelTinyObject(std::string path)
	{

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
		{
			throw std::runtime_error(warn + err);
		}
		for (const auto& shape: shapes)
		{
			for (const auto&  index : shape.mesh.indices)
			{
				Vertex vertex{};
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
				vertex.color = { 1.0f, 1.0f, 1.0f };
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				if (uniqueVertices.count(vertex)==0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
		
		return new VKBufferHandler(myDevice, vertices, indices);
	}

	ModelData ModelLoaderHandler::GetModelVertexAndIndicesTinyObject(std::string path)
	{
		tinyobj::ObjReader reader;
		tinyobj::ObjReaderConfig objConfig;

		if (!reader.ParseFromFile(path, objConfig))
		{
			if (!reader.Error().empty())
			{
				PRINTLVK("Error from reader": )
				PRINTLVK(reader.Error())
			}
			
		}
		if (!reader.Warning().empty()) {
			std::cout << "TinyObjReader: " << reader.Warning();
		}


		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<uint32_t> firstIndices;
		std::vector<uint32_t> firstMeshVertex;
		std::vector<uint32_t> meshIndexCount;
		std::vector<uint32_t> meshVertexCount;
		std::map<int,Material> materialsDatas;
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		std::vector<int> materialIdsOnObject;

		attrib = reader.GetAttrib();
		shapes = reader.GetShapes();
		materials = reader.GetMaterials();
		ModelData modelData={};
		int meshCount = shapes.size();
		int indexStartCounter = 0;
		int vertexStartCouner= 0;
		for (const auto& shape: shapes)
		{
			if (shape.mesh.material_ids[0]<0)
			{
				
				materialIdsOnObject.push_back(0);
			}
			else
			{
				materialIdsOnObject.push_back(static_cast<uint32_t>(shape.mesh.material_ids[0]));
			}
			bool firstIndexAddedPerMesh = false;
			bool firstVertexFinded = false;
			int indexCount = 0;
			int vertexCount = 0;
			for (const auto&  index : shape.mesh.indices)
			{
				Vertex vertex{};
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertex.color = { 1.0f, 1.0f, 1.0f };
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			if (uniqueVertices.count(vertex)==0){
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);

					if (!firstVertexFinded)
					{
						firstMeshVertex.push_back(vertexStartCouner);
						firstVertexFinded = true;
					}
					vertexStartCouner++;
					vertexCount++;
				}
				if (!firstIndexAddedPerMesh)
				{
					firstIndices.push_back(indexStartCounter);
					firstIndexAddedPerMesh = true;
				}
				indices.push_back(uniqueVertices[vertex]);
				indexStartCounter++;
				indexCount++;
				
			}
			meshIndexCount.push_back(indexCount);
			meshVertexCount.push_back(vertexCount);
			
		}

		int textureTotalSize = 0;

		materialsDatas = LoadMaterialsFromObject(path, textureTotalSize);

		std::vector<VKTexture>allTextures;

		modelData = { vertices,indices, firstIndices, firstMeshVertex, materialIdsOnObject, meshIndexCount, meshVertexCount,materialsDatas,allTextures, textureTotalSize, meshCount};
		return modelData;
	}

	std::vector<VKTexture> ModelLoaderHandler::LoadTexturesFromPath(std::string path, VulkanSwapChain& swapChain)
	{
		std::vector<VKTexture> textures;
		std::filesystem::path texturePaths= std::filesystem::path(path);
		if (!std::filesystem::exists(texturePaths)&& !std::filesystem::is_directory(texturePaths))
		{
			PRINTLVK("Not a valid directory or path")	
			return textures;
		}
		std::vector<std::string> paths;
		for (const auto& entry: std::filesystem::directory_iterator(texturePaths))
		{
			paths.push_back(entry.path().string());
		}
		if (paths.size()<=0)
		{
			PRINTLVK("No paths inside the file")	
			return textures;
		}
		for (int i = 0; i < paths.size(); ++i)
		{
			VKTexture texture(paths[i].c_str(), swapChain);
			textures.push_back(texture);
		}
		return textures;
	}


	std::map<int,Material> ModelLoaderHandler:: LoadMaterialsFromObject(std::string path, int& texturesSizes)
	{
		tinyobj::ObjReader reader;
		tinyobj::ObjReaderConfig objConfig;
		std::map<int,Material> materialsDatas;

		if (!reader.ParseFromFile(path, objConfig))
		{
			if (!reader.Error().empty())
			{
				PRINTLVK("Error from reader": )
				PRINTLVK(reader.Error())
			}
			
			return materialsDatas;
		}
		if (!reader.Warning().empty()) {
			std::cout << "TinyObjReader: " << reader.Warning();
		}

		auto& materials = reader.GetMaterials();
		std::unordered_set<std::string> unique_texturePaths;
		
		int matCount = 0;
		for (const auto& material : materials)
		{
			
			Material materialData{};
			materialData.materialUniform.diffuseColor = glm::make_vec3(material.diffuse);
			if (!material.diffuse_texname.empty()) {
				unique_texturePaths.insert(material.diffuse_texname);
			}
			if (!material.specular_texname.empty()) {
				unique_texturePaths.insert(material.specular_texname);
			}
			if (!material.bump_texname.empty()) {
				unique_texturePaths.insert(material.bump_texname);
			}
			if (!material.bump_texname.empty()) {
				unique_texturePaths.insert(material.normal_texname);
			}
			if (!material.bump_texname.empty()) {
				unique_texturePaths.insert(material.ambient_texname);
			}

			materialData.paths = std::vector<std::string>(unique_texturePaths.begin(), unique_texturePaths.end());
			materialData.materialUniform.meshIndex = matCount;
			materialsDatas.try_emplace(matCount, materialData);
			unique_texturePaths.clear();
			matCount++;
		}
		
		return materialsDatas;
	}
}


