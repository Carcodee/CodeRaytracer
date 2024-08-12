#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "imgui.h"

namespace VULKAN {



	struct Vertex {
		glm::vec3 position;
		glm::vec3 color = glm::vec3(1.0f);
		glm::vec3 normal;
        glm::vec3 tangent;
		glm::vec2 texCoord;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();

		bool operator==(const Vertex& other) const {
			return position == other.position && color == other.color && texCoord == other.texCoord && normal == other.normal && tangent == other.tangent;
		}
	};
	struct GLTFVertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 color;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();

		bool operator==(const GLTFVertex& other) const {
			return position == other.position && color == other.color && uv == other.uv && normal==other.normal;
		}
	};

	struct UIVertex {
		ImVec2  pos;
		ImVec2  uv;
		ImU32   col;


		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();

        static std::vector<UIVertex> GetQuad(){
            std::vector<UIVertex> quadVertices = {
                    // Top-left vertex
                    {{-1.0f,  1.0f}, {0.0f, 0.0f}, 0xffffffff},
                    // Bottom-left vertex
                    {{-1.0f, -1.0f}, {0.0f, 1.0f}, 0xffffffff},
                    // Bottom-right vertex
                    {{ 1.0f, -1.0f}, {1.0f, 1.0f}, 0xffffffff},
                    // Top-right vertex
                    {{ 1.0f,  1.0f}, {1.0f, 0.0f}, 0xffffffff}
            };
            return quadVertices;
        }
        static std::vector<uint16_t>GetQuadIndices(){
            std::vector<uint16_t> indices = {
                    0, 1, 2,  // First triangle (top-left, bottom-left, bottom-right)
                    2, 3, 0   // Second triangle (bottom-right, top-right, top-left)
            };
            return indices;
        };



    };

	class VKBufferHandler
	{
	public:
		VKBufferHandler(MyVulkanDevice& device,const std::vector <Vertex>& vertices);
		VKBufferHandler(MyVulkanDevice& device,const std::vector <Vertex>& vertices,const std::vector <uint32_t>& indices);

		VkBuffer vertexBuffer;
		VkBuffer indexBuffer;
		std::vector<uint32_t> indices;
		uint32_t vertexCount;


	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);
		MyVulkanDevice& myDevice;
		VkDeviceMemory vertexBufferMemory;
		VkDeviceMemory indexBufferMemory;


	};


}

namespace std {
	template<> struct hash<VULKAN::Vertex> {
		size_t operator()(VULKAN::Vertex const& vertex) const {
			// Hash the position, color, normal, and texCoord
			size_t hash1 = hash<glm::vec3>()(vertex.position);
			size_t hash2 = hash<glm::vec3>()(vertex.color);
			size_t hash3 = hash<glm::vec3>()(vertex.normal);
            size_t hash4 = hash<glm::vec2>()(vertex.tangent);
			size_t hash5 = hash<glm::vec2>()(vertex.texCoord);

			// Combine the hashes using bitwise operations
			size_t result = hash1;
			result = (result * 31) + hash2;
			result = (result * 31) + hash3;
            result = (result * 31) + hash4;
			result = (result * 31) + hash5;

			return result;
		}
	};
}