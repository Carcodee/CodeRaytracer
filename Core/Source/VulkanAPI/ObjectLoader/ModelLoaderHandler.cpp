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
				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				else {
					vertex.normal = { 0.0f, 0.0f, 0.0f }; // Default normal if not present
				}

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
				PRINTLVK("Error from reader":)
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
		std::map<int, Material> materialsDatas;
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		std::vector<int> materialIdsOnObject;

		attrib = reader.GetAttrib();
		shapes = reader.GetShapes();
		materials = reader.GetMaterials();
		ModelData modelData = {};
		int meshCount = shapes.size();
		int indexStartCounter = 0;
		int vertexStartCouner = 0;
		for (const auto& shape : shapes)
		{
			if (shape.mesh.material_ids.size() <= 0 || shape.mesh.material_ids[0] <= 0)
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
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				else {
					vertex.normal = { 0.0f, 0.0f, 0.0f }; // Default normal if not present
				}

				if (index.texcoord_index == -1)
				{
					vertex.texCoord = {
						0,
						0
					};
				}
				else
				{
					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
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

		materialsDatas = LoadMaterialsFromReader(reader, path, textureTotalSize);

		std::vector<VKTexture>allTextures;

		modelData = { vertices,indices, firstIndices, firstMeshVertex, materialIdsOnObject, meshIndexCount, meshVertexCount,materialsDatas,allTextures, textureTotalSize, meshCount, 0 };
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
		std::filesystem::path currentModelPath(path);
		currentModelPath = currentModelPath.parent_path();

		std::string texturesPath = currentModelPath.string()+"/Textures/";
		if (!std::filesystem::exists(currentModelPath))
		{
			std::cout << "The current model does not have textures relative to the folder: " << path << "\n";
		}

		for (const auto& material : materials)
		{
			
			Material materialData{};
			materialData.materialUniform.diffuseColor = glm::make_vec3(material.diffuse);
			if (!material.diffuse_texname.empty()) {
				std::string texturePathFinded= material.diffuse_texname;
				FixMaterialPaths(texturePathFinded, texturesPath);
				unique_texturePaths.insert(texturePathFinded);
			}
			//if (!material.alpha_texname.empty()) {
			//	std::string texturePathFinded= material.alpha_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}

			//if (!material.specular_texname.empty()) {
			//	std::string texturePathFinded= material.specular_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}
			//if (!material.bump_texname.empty()) {
			//	std::string texturePathFinded= material.bump_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}
			//if (!material.normal_texname.empty()) {
			//	std::string texturePathFinded= material.normal_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}
			//if (!material.ambient_texname.empty()) {
			//	std::string texturePathFinded= material.ambient_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}

			materialData.paths = std::vector<std::string>(unique_texturePaths.begin(), unique_texturePaths.end());
			materialData.materialUniform.meshIndex = matCount;
			materialsDatas.try_emplace(matCount, materialData);
			unique_texturePaths.clear();
			matCount++;
		}
		
		return materialsDatas;
	}

	std::map<int, Material> ModelLoaderHandler::LoadMaterialsFromReader(tinyobj::ObjReader reader ,std::string path, int& texturesSizes)
	{
		auto& materials = reader.GetMaterials();
		std::unordered_set<std::string> unique_texturePaths;

		int matCount = 0;
		std::filesystem::path currentModelPath(path);
		currentModelPath = currentModelPath.parent_path();

		std::string texturesPath = currentModelPath.string() + "/textures/";
		if (!std::filesystem::exists(currentModelPath))
		{
			std::cout << "The current model does not have textures relative to the folder: " << path << "\n";
		}

		std::map<int,Material> materialsDatas;

		for (const auto& material : materials)
		{

			Material materialData{};
			materialData.materialUniform.diffuseColor = glm::make_vec3(material.diffuse);
			if (!material.diffuse_texname.empty()) {
				std::string texturePathFinded = material.diffuse_texname;
				FixMaterialPaths(texturePathFinded, texturesPath);
				unique_texturePaths.insert(texturePathFinded);
			}
			//if (!material.alpha_texname.empty()) {
			//	std::string texturePathFinded = material.alpha_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}

			//if (!material.specular_texname.empty()) {
			//	std::string texturePathFinded= material.specular_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}
			//if (!material.bump_texname.empty()) {
			//	std::string texturePathFinded= material.bump_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}
			//if (!material.normal_texname.empty()) {
			//	std::string texturePathFinded= material.normal_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}
			//if (!material.ambient_texname.empty()) {
			//	std::string texturePathFinded= material.ambient_texname;
			//	FixMaterialPaths(texturePathFinded, texturesPath);
			//	unique_texturePaths.insert(texturePathFinded);
			//}

			materialData.paths = std::vector<std::string>(unique_texturePaths.begin(), unique_texturePaths.end());
			materialData.materialUniform.meshIndex = matCount;
			materialsDatas.try_emplace(matCount, materialData);
			unique_texturePaths.clear();
			matCount++;
		}

		return materialsDatas;
	}

	void ModelLoaderHandler::FixMaterialPaths(std::string& path, std::string texturesPath)
	{
		if (!std::filesystem::exists(texturesPath))
		{
			std::cout << "Filepath for textures does not exist: " << texturesPath << "\n";
			path = "";
			return;
			
		}
		if (!std::filesystem::exists(path)&&path.size()>0)
		{
			size_t notValidPathFinishPos = path.find_last_of("//");
			path.erase(0, notValidPathFinishPos + 1);
			size_t extensionPart = path.find_last_of('.');
			path.erase(extensionPart, path.size());
			for (auto element :std::filesystem::directory_iterator(texturesPath))
			{
				if (element.is_regular_file())
				{
					std::string fileInPath=element.path().string();
					std::string bufferFileInPath=element.path().string();

					size_t filePathFinishPos = fileInPath.find_last_of("//");

					fileInPath.erase(0, filePathFinishPos + 1);
					bufferFileInPath.erase(0, filePathFinishPos + 1);

					size_t extensionPartFileInPath = bufferFileInPath.find_last_of('.');

					//extension part
					bufferFileInPath.erase(extensionPartFileInPath, path.size());

					if (bufferFileInPath == path)
					{
						path = fileInPath;
						std::cout << "Filepath matches with: " << path <<"\n";
						break;
					}
					
				}
				
			}
			path = texturesPath + path;
			if (!std::filesystem::exists(path))
			{
				std::cout << "Filepath does not exist: " << path <<"\n";
				path = "";
			}

		}
		else
		{
			std::cout << "Path is absolute and exist: " << path <<"\n";
		}
	}
}


