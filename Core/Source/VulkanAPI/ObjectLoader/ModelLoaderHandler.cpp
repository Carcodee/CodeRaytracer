#include "ModelLoaderHandler.h"
#include "VulkanAPI/Model/ModelHandler.h"
#include <filesystem>
#include <fstream>
#include <unordered_set>

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE

namespace VULKAN {

	ModelLoaderHandler *ModelLoaderHandler::instance = nullptr;



	ModelLoaderHandler* ModelLoaderHandler::GetInstance()
	{
		if (instance==nullptr)
		{
			instance = new ModelLoaderHandler();
		}
		return instance;
	}

	ModelLoaderHandler::ModelLoaderHandler()
	{

	}

    void ModelLoaderHandler::FindReader(tinyobj::ObjReader& reader, std::string path) {
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
        

        
		attrib = reader.GetAttrib();
		shapes = reader.GetShapes();
		materials = reader.GetMaterials();

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> firstIndices;
        std::vector<uint32_t> firstMeshVertex;
        std::vector<uint32_t> meshIndexCount;
        std::vector<uint32_t> meshVertexCount;
        std::map<int, Material> materialsDatas;
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        std::vector<int> materialIdsOnObject;
        int meshCount = shapes.size();
        int indexStartCounter = 0;
        int vertexStartCouner = 0;
        
		for (const auto& shape : shapes)
		{
			if (shape.mesh.material_ids.size() <= 0 || shape.mesh.material_ids[0] < 0)
			{
				materialIdsOnObject.push_back(0);
			}
			else
			{
				materialIdsOnObject.push_back(shape.mesh.material_ids[0]+ ModelHandler::GetInstance()->currentMaterialsOffset);
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
        for (int i = 0; i <indices.size() ; i+=3) {

            int index1 =indices[i];
            int index2 =indices[i + 1];
            int index3 =indices[i + 2];

            glm::vec3 tangent = CalculateTangent(vertices[index1].position,vertices[index2].position,vertices[index3].position,
                                                 vertices[index1].texCoord,vertices[index2].texCoord,vertices[index3].texCoord);
            vertices[index1].tangent = tangent;
            vertices[index2].tangent = tangent;
            vertices[index3].tangent = tangent;
        }
        int textureTotalSize = 0;

        ModelData modelData = {};
        modelData.materialOffset= ModelHandler::GetInstance()->currentMaterialsOffset;
		materialsDatas = LoadMaterialsFromReader(reader, path);
		std::vector<VKTexture>allTextures;

        modelData.vertices=vertices;
        modelData.indices=indices;
        modelData.firstMeshIndex=firstIndices;
        modelData.firstMeshVertex=firstMeshVertex;
        modelData.materialIds=materialIdsOnObject;
        modelData.meshIndexCount=meshIndexCount;
        modelData.meshVertexCount=meshVertexCount;
        modelData.materialDataPerMesh=materialsDatas;
        modelData.meshCount = meshCount;
        modelData.indexBLASOffset = 0;
        modelData.vertexBLASOffset = 0;
        modelData.transformBLASOffset = 0;
        modelData.generated = true;
        modelData.modelFormat = OBJ;
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

	std::map<int, Material> ModelLoaderHandler::LoadMaterialsFromReader(tinyobj::ObjReader& reader ,std::string path)
	{
		auto& materials = reader.GetMaterials();

        std::map<int,Material> materialsDatas;

		int matCount = 0;
		std::filesystem::path currentModelPath(path);
		currentModelPath = currentModelPath.parent_path();

		std::string texturesPath = currentModelPath.string() + "\\textures";
		if (!std::filesystem::exists(texturesPath))
		{
			std::cout << "The current model does not have textures relative to the folder, Creating materials at model path: " << path << "\n";
		}


		for (const auto& material : materials)
		{
			Material materialData{};
			materialData.materialUniform.diffuseColor = glm::make_vec3(material.diffuse);
            materialData.materialUniform.roughnessIntensity = material.roughness;
            materialData.materialUniform.metallicIntensity = material.metallic;
            bool texturesFinded= false;
			if (!material.diffuse_texname.empty()) {
				std::string texturePathFinded = material.diffuse_texname;
				FixMaterialPaths(texturePathFinded, texturesPath, currentModelPath.string());
                materialData.paths.try_emplace(TEXTURE_TYPE::DIFFUSE,texturePathFinded);
                materialData.materialUniform.diffuseColor = glm::vec3(1.0f);
                texturesFinded = true;
			}
            if (!material.roughness_texname.empty() || !material.specular_texname.empty()|| !material.specular_highlight_texname.empty()) {
                std::string texName;
                if (!material.roughness_texname.empty()){
                    texName=material.roughness_texname;
                }else if (!material.specular_texname.empty()){
                    texName=material.specular_texname;
                }else if (!material.specular_highlight_texname.empty()){
                    texName=material.specular_highlight_texname;
                }
                std::string texturePathFinded= texName;
                FixMaterialPaths(texturePathFinded, texturesPath, currentModelPath.string());
                materialData.paths.try_emplace(TEXTURE_TYPE::ROUGHNESS,texturePathFinded);
                materialData.materialUniform.roughnessIntensity = 0.5f;
                texturesFinded = true;
            }
            
            if (!material.metallic_texname.empty()) {
                std::string texturePathFinded= material.metallic_texname;
                FixMaterialPaths(texturePathFinded, texturesPath, currentModelPath.string());
                materialData.paths.try_emplace(TEXTURE_TYPE::METALLIC,texturePathFinded);
                materialData.materialUniform.metallicIntensity = 1.0f;
                texturesFinded = true;
            }

			if (!material.bump_texname.empty()) {
				std::string texturePathFinded= material.bump_texname;
				FixMaterialPaths(texturePathFinded, texturesPath, currentModelPath.string());
                materialData.paths.try_emplace(TEXTURE_TYPE::NORMAL,texturePathFinded);
                materialData.materialUniform.normalIntensity = 1.0f;
                texturesFinded = true;
			}

            if (!material.emissive_texname.empty()) {
                std::string texturePathFinded= material.emissive_texname;
                FixMaterialPaths(texturePathFinded, texturesPath, currentModelPath.string());
                materialData.paths.try_emplace(TEXTURE_TYPE::EMISSIVE,texturePathFinded);
                materialData.materialUniform.emissionIntensity = 1.0f;
                texturesFinded = true;
            }
            materialData.textureReferencePath= texturesPath;
            materialData.name = "Material_"+std::to_string(matCount);
            materialData.id = ModelHandler::GetInstance()->currentMaterialsOffset;
            if(texturesFinded){
                materialData.targetPath = texturesPath +"\\"+ materialData.name + ".MATCODE";
            } else{
                materialData.targetPath = currentModelPath.string() +"\\"+ materialData.name + ".MATCODE";
            }
			materialsDatas.try_emplace(matCount, materialData);
            ModelHandler::GetInstance()->allMaterialsOnApp.try_emplace(materialData.id,std::make_shared<Material>(materialData));
			matCount++;
            ModelHandler::GetInstance()->currentMaterialsOffset++;
		}

		return materialsDatas;
	}

	void ModelLoaderHandler::FixMaterialPaths(std::string& path, std::string texturesPath, std::string modelPath)
	{
		if (!std::filesystem::exists(texturesPath))
		{
			path =modelPath;
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

					size_t filePathFinishPos = fileInPath.find_last_of("\\");

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
			path = texturesPath + "\\" + path;
			if (!std::filesystem::exists(path))
			{
				std::cout << "Filepath does not exist: " << path <<"\n";
				path = modelPath;
			}

		}
		else
		{
			std::cout << "Path is absolute and exist: " << path <<"\n";
		}
	}

    void ModelLoaderHandler::GetModelFromReader(tinyobj::ObjReader& reader, ModelData& modelData) {

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
        int meshCount = shapes.size();
        int indexStartCounter = 0;
        int vertexStartCouner = 0;
        for (const auto& shape : shapes)
        {
            if (shape.mesh.material_ids.size() <= 0 || shape.mesh.material_ids[0] < 0)
            {
                materialIdsOnObject.push_back(0);
            }
            else
            {
                materialIdsOnObject.push_back(shape.mesh.material_ids[0] + modelData.materialOffset);
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
        for (int i = 0; i <indices.size() ; i+=3) {

            int index1 =indices[i];
            int index2 =indices[i + 1];
            int index3 =indices[i + 2];
            
            glm::vec3 tangent = CalculateTangent(vertices[index1].position,vertices[index2].position,vertices[index3].position,
                                                 vertices[index1].texCoord,vertices[index2].texCoord,vertices[index3].texCoord);
            vertices[index1].tangent = tangent;
            vertices[index2].tangent = tangent;
            vertices[index3].tangent = tangent;
        } 
        int textureTotalSize = 0;
        std::vector<VKTexture>allTextures;
        modelData.vertices=vertices;
        modelData.indices=indices;
        modelData.firstMeshIndex=firstIndices;
        modelData.firstMeshVertex=firstMeshVertex;
        modelData.materialIds=materialIdsOnObject;
        modelData.meshIndexCount=meshIndexCount;
        modelData.meshVertexCount=meshVertexCount;
        modelData.materialDataPerMesh=materialsDatas;
        modelData.meshCount = meshCount;
        modelData.indexBLASOffset = 0;
        modelData.vertexBLASOffset = 0;
        modelData.transformBLASOffset = 0;
        modelData.generated = true;
        
    }


    glm::vec3 ModelLoaderHandler::CalculateTangent(glm::vec3& pos1, glm::vec3& pos2, glm::vec3& pos3,
                                                   glm::vec2& uv1, glm::vec2& uv2, glm::vec2& uv3) {
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;
        glm::vec3 tangent;
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        return glm::normalize(tangent);
    }

    void ModelLoaderHandler::LoadGLTFModel(std::string path,  tinygltf::Model& model, ModelData& modelData) {

        std::string err;
        std::string warn;
        tinygltf::TinyGLTF gltfContext; 
        gltfContext.LoadASCIIFromFile(&model, &err, &warn, path);
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> firstIndices;
        std::vector<uint32_t> firstMeshVertex;
        std::vector<uint32_t> meshIndexCount;
        std::vector<uint32_t> meshVertexCount;
//        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        std::vector<int> materialIdsOnObject;
        std::vector<glm::mat4> matrices;
        NodeChain nodeChain{};
        int meshCount = model.meshes.size();
        
        int indexStartCounter = 0;
        int vertexStartCouner = 0;
        
        for(auto& scene : model.scenes){
            for (int i = 0; i < scene.nodes.size(); ++i) {
                LoadGLTFNode(model,
                             &model.nodes[scene.nodes[i]],
                             &nodeChain,
                             indices,
                             vertices,
                             firstIndices,
                             firstMeshVertex,
                             meshIndexCount,
                             meshVertexCount,
                             materialIdsOnObject,
                             matrices,
                             meshCount);

            }
           
        }

        std::vector<VKTexture>allTextures;

        std::filesystem::path modelPath (std::filesystem::path(path).parent_path());
        LoadGLTFMaterials(model,modelData.materialDataPerMesh,modelPath.string());
        modelData.vertices=vertices;
        modelData.indices=indices;
        modelData.firstMeshIndex=firstIndices;
        modelData.firstMeshVertex=firstMeshVertex;
        modelData.materialIds=materialIdsOnObject;
        modelData.meshIndexCount=meshIndexCount;
        modelData.meshVertexCount=meshVertexCount;
        modelData.materialDataPerMesh=modelData.materialDataPerMesh;
        modelData.meshCount = meshCount;
        modelData.indexBLASOffset = 0;
        modelData.vertexBLASOffset = 0;
        modelData.transformBLASOffset = 0;
        modelData.generated = true;
        modelData.modelFormat = GLTF;
        modelData.matrices = matrices; 
    }

    //removing all node transformation
    void ModelLoaderHandler::LoadGLTFNode(tinygltf::Model& model,
                                          tinygltf::Node* node,
                                          NodeChain* nodeParent,
                                          std::vector<uint32_t>& indices,
                                          std::vector<Vertex>& vertices,
                                          std::vector<uint32_t>& firstMeshIndices,
                                          std::vector<uint32_t>& firstMeshVertces,
                                          std::vector<uint32_t>& meshIndexCount,
                                          std::vector<uint32_t>& meshVertexCount,
                                          std::vector<int>& materialsIds,
                                          std::vector<glm::mat4>& matrices,
                                          int& meshCount) {



        glm::mat4 nodeMat = glm::mat4(1.0f);
        if(node->scale.size()==3){
            nodeMat = glm::scale(nodeMat,glm::vec3 (glm::make_vec3(node->scale.data())));
        }
        if(node->rotation.size()==4){
            glm::quat rot = glm::make_quat(node->rotation.data());
            nodeMat *= glm::mat4(rot);
        }
        if(node->translation.size()==3){
            nodeMat = glm::translate(nodeMat,glm::vec3 (glm::make_vec3(node->translation.data())));
        }
        if(node->matrix.size()==16){
            nodeMat = glm::make_mat4(node->matrix.data());
        }
        NodeChain currentNodeChain;
        currentNodeChain.parent = nodeParent;
        currentNodeChain.node = node;
        currentNodeChain.matrix = nodeMat;

        if(!node->children.empty()){
            for (int i = 0; i < node->children.size(); ++i) {
                tinygltf::Node* childNode= &model.nodes[node->children[i]];
                LoadGLTFNode(model,
                             childNode,
                             &currentNodeChain,
                             indices,
                             vertices,
                             firstMeshIndices,
                             firstMeshVertces,
                             meshIndexCount,
                             meshVertexCount,
                             materialsIds,
                             matrices,
                             meshCount);

            }
        }
        if (node->mesh > -1){
            TransformNodeToWorld(nodeParent, nodeMat);
            matrices.push_back(nodeMat);
            tinygltf::Mesh currentMesh = model.meshes[node->mesh];
            firstMeshIndices.push_back(indices.size());
            firstMeshVertces.push_back(vertices.size());
            uint32_t indexStartCounter = indices.size();
            uint32_t vertexStartCounter = vertices.size();
            if (currentMesh.primitives[0].material > -1)
            {
                materialsIds.push_back(currentMesh.primitives[0].material+ ModelHandler::GetInstance()->currentMaterialsOffset);
            }
            else
            {
                materialsIds.push_back(0);
            }
            for (int i = 0; i < currentMesh.primitives.size(); ++i) {
                tinygltf::Primitive primitive = currentMesh.primitives[i];
                uint32_t indexCount = 0;
                uint32_t vertexCount = 0;
                //vertices
                {
                    const float* positionBuffer = nullptr;
                    const float* normalBuffer = nullptr;
                    const float* textCoordBuffer = nullptr;
                    const float* tangentBuffer = nullptr;

                    if(primitive.attributes.find("POSITION") != primitive.attributes.end()){
                        tinygltf::Accessor& accessor =model.accessors[primitive.attributes.find("POSITION")->second];
                        tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                        positionBuffer = reinterpret_cast<const float*>(&(model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
                        vertexCount = accessor.count;

                    }
                    if(primitive.attributes.find("NORMAL") != primitive.attributes.end()){
                        tinygltf::Accessor& accessor =model.accessors[primitive.attributes.find("NORMAL")->second];
                        tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                        normalBuffer = reinterpret_cast<const float*>(&(model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));

                    }
                    if(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()){
                        tinygltf::Accessor& accessor =model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                        tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                        textCoordBuffer = reinterpret_cast<const float*>(&(model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));

                    }
                    if(primitive.attributes.find("TANGENT") != primitive.attributes.end()){
                        tinygltf::Accessor& accessor =model.accessors[primitive.attributes.find("TANGENT")->second];
                        tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                        tangentBuffer = reinterpret_cast<const float*>(&(model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
   
                    }
                    glm::mat4 transposedMat = glm::transpose(nodeMat);
                    for (size_t j = 0; j < vertexCount; ++j) {
                        Vertex vertex{};
                        vertex.position = glm::make_vec3(&positionBuffer[j * 3]);

                        //todo: fix normal space :D
                        vertex.normal =normalBuffer ? glm::normalize(glm::make_vec3(&normalBuffer[j * 3])): glm::vec3 (1.0f);
                        glm::vec4 result = nodeMat * glm::vec4 (vertex.normal.x,vertex.normal.y,vertex.normal.z,1);
                        vertex.normal =glm::vec3(result.x,result.y,result.z);
                        
                        vertex.texCoord =textCoordBuffer? glm::make_vec3(&textCoordBuffer[j * 2]): glm::vec2 (0.0f);
                        vertex.color = glm::vec3 (1.0f);
                        
                        vertex.tangent = tangentBuffer? glm::normalize(glm::make_vec3(&tangentBuffer[j * 3])): glm::vec3 (1.0f);
                        glm::vec4 resultTang = nodeMat * glm::vec4 (vertex.tangent.x,vertex.tangent.y,vertex.tangent.z,1);
                        vertex.tangent =glm::vec3(resultTang.x,resultTang.y,resultTang.z);
                        
                        vertices.push_back(vertex);
                    }
                    meshVertexCount.push_back(vertexCount);

                }
                //indices
                {
                    tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                    tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    indexCount+= static_cast<uint32_t>(accessor.count);
                    meshIndexCount.push_back(indexCount);
                    switch (accessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            const uint32_t* buff = reinterpret_cast<const uint32_t*>(&(model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
                            for (size_t j = 0; j <accessor.count; ++j) {
                                indices.push_back(buff[j] + vertexStartCounter);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            const uint16_t* buff = reinterpret_cast<const uint16_t*>(&(model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
                            for (size_t j = 0; j <accessor.count; ++j) {
                                indices.push_back(buff[j]  + vertexStartCounter);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            const uint8_t* buff = reinterpret_cast<const uint8_t*>(&(model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
                            for (size_t j = 0; j <accessor.count; ++j) {
                                indices.push_back(buff[j] + vertexStartCounter);
                            }
                            break;
                        }
                        default:
                            std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                            return;
                    }

                }

            }

        }
    }
    void ModelLoaderHandler::TransformNodeToWorld(ModelLoaderHandler::NodeChain *nodeChainParent, glm::mat4& mat) {
        NodeChain* currentParent =nodeChainParent;
        while (currentParent != nullptr){
            mat =  currentParent->matrix * mat;
            currentParent = currentParent->parent;
        }
    }
    void ModelLoaderHandler::LoadGLTFMaterials(tinygltf::Model &model,std::map<int,Material>&materialDataPerMesh, std::string modelPath) {
        
        std::string texturesPath = modelPath+"\\"+"textures";
        if (!std::filesystem::exists(texturesPath)){
            texturesPath = modelPath; 
        }
        for (int i = 0; i <model.materials.size() ; ++i) {

            bool texturesFinded = false;
            const tinygltf::Material& gltfMaterial = model.materials[i];

            Material material;
            
            material.materialUniform.diffuseColor = glm::vec3 (gltfMaterial.pbrMetallicRoughness.baseColorFactor[0],
                                                               gltfMaterial.pbrMetallicRoughness.baseColorFactor[1],
                                                               gltfMaterial.pbrMetallicRoughness.baseColorFactor[2]);
            material.materialUniform.roughnessIntensity = gltfMaterial.pbrMetallicRoughness.roughnessFactor;
            material.materialUniform.metallicIntensity = gltfMaterial.pbrMetallicRoughness.metallicFactor;
            
            if(gltfMaterial.pbrMetallicRoughness.baseColorTexture.index>-1){
                std::string path = GetGltfTexturePath(modelPath, model.images[model.textures[gltfMaterial.pbrMetallicRoughness.baseColorTexture.index].source].uri);
                if (path!=""){
                    material.paths.try_emplace(TEXTURE_TYPE::DIFFUSE,path);
                    material.materialUniform.diffuseColor = glm::vec3(1.0f);
                    material.materialUniform.albedoIntensity = 1.0f;
                    texturesFinded = true;       
                }               
            }
            if(gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index>-1){
                std::string path = GetGltfTexturePath(modelPath, model.images[model.textures[gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index].source].uri);
                if (path!=""){
                    material.paths.try_emplace(TEXTURE_TYPE::METALLICROUGHNESS,path);
                    material.materialUniform.roughnessIntensity =  static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
                    material.materialUniform.metallicIntensity = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
                    texturesFinded = true;
                }
            }
            if(gltfMaterial.normalTexture.index>-1){
                std::string path = GetGltfTexturePath(modelPath, model.images[model.textures[gltfMaterial.normalTexture.index].source].uri);
                if (path!=""){
                    material.paths.try_emplace(TEXTURE_TYPE::NORMAL,path);
                    material.materialUniform.normalIntensity = 1.0f;
                }
            }
            if(gltfMaterial.emissiveTexture.index>-1){
                std::string path = GetGltfTexturePath(modelPath, model.images[model.textures[gltfMaterial.emissiveTexture.index].source].uri);
                if (path!=""){
                    material.paths.try_emplace(TEXTURE_TYPE::EMISSIVE,path);
                    material.materialUniform.emissionIntensity = 1.0f;
                }

            }
            material.textureReferencePath= texturesPath;
            material.name = "Material_"+std::to_string(i);
            material.id = ModelHandler::GetInstance()->currentMaterialsOffset;
            material.targetPath = texturesPath +"\\"+ material.name + ".MATCODE";

            materialDataPerMesh.try_emplace(i, material);
            ModelHandler::GetInstance()->allMaterialsOnApp.try_emplace(material.id,std::make_shared<Material>(material));
            ModelHandler::GetInstance()->currentMaterialsOffset++;
        }
    }

    std::string ModelLoaderHandler::GetGltfTexturePath(std::string modelPath, std::string uriPath) {
        if (std::filesystem::exists(uriPath)){
            return uriPath;
        }
        std::string path =modelPath +"\\"+ uriPath;
        if (std::filesystem::exists(path)){
            return path;
        }
        std::cout<< "there is no valid texture path on the gltf path: "<<modelPath <<"\n"; 
        return "";
    }

    void ModelLoaderHandler::GetGLTFModel( tinygltf::Model &model,std::string path) {
        std::string err;
        std::string warn;
        tinygltf::TinyGLTF gltfContext;
        gltfContext.LoadASCIIFromFile(&model, &err, &warn, path);
    }

    void ModelLoaderHandler::LoadGLTFFromModel(tinygltf::Model &model, ModelData &modelData) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> firstIndices;
        std::vector<uint32_t> firstMeshVertex;
        std::vector<uint32_t> meshIndexCount;
        std::vector<uint32_t> meshVertexCount;
//        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        std::vector<int> materialIdsOnObject;
        std::vector<glm::mat4> matrices;
        NodeChain nodeChain{};
        int meshCount = model.meshes.size();

        int indexStartCounter = 0;
        int vertexStartCouner = 0;

        for(auto& scene : model.scenes){
            for (int i = 0; i < scene.nodes.size(); ++i) {
                LoadGLTFNode(model,
                             &model.nodes[scene.nodes[i]],
                             &nodeChain,
                             indices,
                             vertices,
                             firstIndices,
                             firstMeshVertex,
                             meshIndexCount,
                             meshVertexCount,
                             materialIdsOnObject,
                             matrices,
                             meshCount);

            }

        }

        std::vector<VKTexture>allTextures;
        modelData.vertices=vertices;
        modelData.indices=indices;
        modelData.firstMeshIndex=firstIndices;
        modelData.firstMeshVertex=firstMeshVertex;
        modelData.materialIds=materialIdsOnObject;
        modelData.meshIndexCount=meshIndexCount;
        modelData.meshVertexCount=meshVertexCount;
        modelData.meshCount = meshCount;
        modelData.indexBLASOffset = 0;
        modelData.vertexBLASOffset = 0;
        modelData.transformBLASOffset = 0;
        modelData.generated = true;
        modelData.modelFormat = GLTF;
        modelData.matrices = matrices;

    }


}

