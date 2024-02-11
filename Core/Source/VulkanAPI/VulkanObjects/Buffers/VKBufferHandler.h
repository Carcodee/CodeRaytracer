#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"


namespace VULKAN {

	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;
		glm::vec2 texCoord;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();

	};
	class VKBufferHandler
	{
	public:
		VKBufferHandler(MyVulkanDevice& device,const std::vector <Vertex>& vertices);
		VKBufferHandler(MyVulkanDevice& device,const std::vector <Vertex>& vertices,const std::vector <uint16_t>& indices);

		VkBuffer vertexBuffer;
		VkBuffer indexBuffer;
		std::vector<uint16_t> indices;
		uint32_t vertexCount;


	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint16_t>& indices);
		MyVulkanDevice& myDevice;
		VkDeviceMemory vertexBufferMemory;
		VkDeviceMemory indexBufferMemory;


	};


}

