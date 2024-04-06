#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "imgui.h"

namespace VULKAN {



	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();

		bool operator==(const Vertex& other) const {
			return position == other.position && color == other.color && texCoord == other.texCoord;
		}
		

	};


	struct UIVertex {
		ImVec2  pos;
		ImVec2  uv;
		ImU32   col;


		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();


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
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}