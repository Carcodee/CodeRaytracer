#pragma once

#include <tiny_obj_loader.h>
#include <tiny_gltf.h>
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/VulkanObjects/Buffers/VKBufferHandler.h"
#include <unordered_map>
#include "VulkanAPI/Utility/Utility.h"
#include <vector>
#include <string>
#include "VulkanAPI/VulkanObjects/Textures/VKTexture.h"
#include <glm/gtc/type_ptr.hpp>

namespace VULKAN {
	class ModelLoaderHandler
	{

		
	protected:
		static ModelLoaderHandler* instance;

        struct NodeChain{
            NodeChain* parent = nullptr;
            tinygltf::Node* node = nullptr;
            glm::mat4 matrix = glm::mat4 (1.0f);
        };

	public:
		
		static ModelLoaderHandler* GetInstance();
		ModelLoaderHandler();



        void FindReader(tinyobj::ObjReader& reader,std::string path);
		ModelData GetModelVertexAndIndicesTinyObject(std::string path);
        void GetModelFromReader(tinyobj::ObjReader& reader, ModelData& modelData);
		std::vector<VKTexture> LoadTexturesFromPath(std::string path,VulkanSwapChain& swapChain);
		std::map<int,Material> LoadMaterialsFromReader(tinyobj::ObjReader& reader,std::string path);
		void FixMaterialPaths(std::string& path, std::string texturesPath, std::string modelPath);
        glm::vec3 CalculateTangent(glm::vec3& pos1, glm::vec3& pos2,glm::vec3& pos3,
                                    glm::vec2& uv1, glm::vec2& uv2,glm::vec2& uv3);

        void LoadGLTFModel(std::string path, tinygltf::Model& model, ModelData& modelData);
        void LoadGLTFMaterials(tinygltf::Model& model,std::map<int,Material>&materialDataPerMesh ,std::string modelPath);
        void TransformNodeToWorld(NodeChain* nodeChainParent, glm::mat4& mat);
        std::string GetGltfTexturePath(std::string modelPath, std::string uriPath);
        void LoadGLTFNode(tinygltf::Model& model,
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
                          int& meshCount);
	};

	//class GLTFObject
	//{
	//	struct Primitive {
	//		uint32_t firstIndex;
	//		uint32_t indexCount;
	//		int32_t materialIndex;
	//	};

	//	// Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
	//	struct Mesh {
	//		std::vector<Primitive> primitives;
	//	};

	//	// A node represents an object in the glTF scene graph
	//	struct Node {
	//		Node* parent;
	//		std::vector<Node*> children;
	//		Mesh mesh;
	//		glm::mat4 matrix;
	//		~Node() {
	//			for (auto& child : children) {
	//				delete child;
	//			}
	//		}
	//	};
	//	struct Material {
	//		glm::vec4 baseColorFactor = glm::vec4(1.0f);
	//		uint32_t baseColorTextureIndex;
	//	};
	//	struct Image {
	//		// We also store (and create) a descriptor set that's used to access this texture from the fragment shader
	//		VkDescriptorSet descriptorSet;
	//	};
	//	struct Texture {
	//		int32_t imageIndex;
	//	};
	//	std::vector<Image> images;
	//	std::vector<Texture> textures;
	//	std::vector<Material> materials;
	//	std::vector<Node*> nodes;

	//	void loadImages(tinygltf::Model& input)
	//	{
	//		// Images can be stored inside the glTF (which is the case for the sample model), so instead of directly
	//		// loading them from disk, we fetch them from the glTF loader and upload the buffers
	//		images.resize(input.images.size());
	//		for (size_t i = 0; i < input.images.size(); i++) {
	//			tinygltf::Image& glTFImage = input.images[i];
	//			// Get the image data from the glTF loader
	//			unsigned char* buffer = nullptr;
	//			VkDeviceSize bufferSize = 0;
	//			bool deleteBuffer = false;
	//			// We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
	//			if (glTFImage.component == 3) {
	//				bufferSize = glTFImage.width * glTFImage.height * 4;
	//				buffer = new unsigned char[bufferSize];
	//				unsigned char* rgba = buffer;
	//				unsigned char* rgb = &glTFImage.image[0];
	//				for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) {
	//					memcpy(rgba, rgb, sizeof(unsigned char) * 3);
	//					rgba += 4;
	//					rgb += 3;
	//				}
	//				deleteBuffer = true;
	//			}
	//			else {
	//				buffer = &glTFImage.image[0];
	//				bufferSize = glTFImage.image.size();
	//			}
	//			// Load texture from image buffer
	//			if (deleteBuffer) {
	//				delete[] buffer;
	//			}
	//		}
	//	}
	//	void loadTextures(tinygltf::Model& input)
	//	{
	//		textures.resize(input.textures.size());
	//		for (size_t i = 0; i < input.textures.size(); i++) {
	//			textures[i].imageIndex = input.textures[i].source;
	//		}
	//	}
	//	void loadMaterials(tinygltf::Model& input)
	//	{
	//		materials.resize(input.materials.size());
	//		for (size_t i = 0; i < input.materials.size(); i++) {
	//			// We only read the most basic properties required for our sample
	//			tinygltf::Material glTFMaterial = input.materials[i];
	//			// Get the base color factor
	//			if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
	//				materials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
	//			// Get base color texture index
	//				if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
	//					materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
	//				}
	//			}
	//		}

	//	};

	//	void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<GLTFVertex>& vertexBuffer)
	//	{
	//		Node* node = new Node{};
	//		node->matrix = glm::mat4(1.0f);
	//		node->parent = parent;

	//		// Get the local node matrix
	//		// It's either made up from translation, rotation, scale or a 4x4 matrix
	//		if (inputNode.translation.size() == 3) {
	//			node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
	//		}
	//		if (inputNode.rotation.size() == 4) {
	//			glm::quat q = glm::make_quat(inputNode.rotation.data());
	//			node->matrix *= glm::mat4(q);
	//		}
	//		if (inputNode.scale.size() == 3) {
	//			node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
	//		}
	//		if (inputNode.matrix.size() == 16) {
	//			node->matrix = glm::make_mat4x4(inputNode.matrix.data());
	//		};

	//		// Load node's children
	//		if (inputNode.children.size() > 0) {
	//			for (size_t i = 0; i < inputNode.children.size(); i++) {
	//				loadNode(input.nodes[inputNode.children[i]], input, node, indexBuffer, vertexBuffer);
	//			}
	//		}

	//		if (inputNode.mesh > -1) {
	//			const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
	//			// Iterate through all primitives of this node's mesh
	//			for (size_t i = 0; i < mesh.primitives.size(); i++) {
	//				const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
	//				uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
	//				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
	//				uint32_t indexCount = 0;
	//				// Vertices
	//				{
	//					const float* positionBuffer = nullptr;
	//					const float* normalsBuffer = nullptr;
	//					const float* texCoordsBuffer = nullptr;
	//					size_t vertexCount = 0;

	//					// Get buffer data for vertex positions
	//					if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
	//						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
	//						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
	//						positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
	//						vertexCount = accessor.count;
	//					}
	//					// Get buffer data for vertex normals
	//					if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
	//						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
	//						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
	//						normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
	//					}
	//					// Get buffer data for vertex texture coordinates
	//					// glTF supports multiple sets, we only load the first one
	//					if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
	//						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
	//						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
	//						texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
	//					}

	//					// Append data to model's vertex buffer
	//					for (size_t v = 0; v < vertexCount; v++) {
	//						GLTFVertex vert{};
	//						vert.position = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
	//						vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
	//						vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
	//						vert.color = glm::vec3(1.0f);
	//						vertexBuffer.push_back(vert);
	//					}
	//				}
	//				// Indices
	//				{
	//					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
	//					const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
	//					const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

	//					indexCount += static_cast<uint32_t>(accessor.count);

	//					// glTF supports different component types of indices
	//					switch (accessor.componentType) {
	//					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
	//						const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
	//						for (size_t index = 0; index < accessor.count; index++) {
	//							indexBuffer.push_back(buf[index] + vertexStart);
	//						}
	//						break;
	//					}
	//					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
	//						const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
	//						for (size_t index = 0; index < accessor.count; index++) {
	//							indexBuffer.push_back(buf[index] + vertexStart);
	//						}
	//						break;
	//					}
	//					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
	//						const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
	//						for (size_t index = 0; index < accessor.count; index++) {
	//							indexBuffer.push_back(buf[index] + vertexStart);
	//						}
	//						break;
	//					}
	//					default:
	//						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
	//						return;
	//					}
	//				}
	//				Primitive primitive{};
	//				primitive.firstIndex = firstIndex;
	//				primitive.indexCount = indexCount;
	//				primitive.materialIndex = glTFPrimitive.material;
	//				node->mesh.primitives.push_back(primitive);
	//			}
	//		}

	//		if (parent) {
	//			parent->children.push_back(node);
	//		}
	//		else {
	//			nodes.push_back(node);
	//		}
	//	}

	//};

}


