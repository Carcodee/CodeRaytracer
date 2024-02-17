#include "VKBufferHandler.h"

namespace VULKAN {


	VKBufferHandler::VKBufferHandler(MyVulkanDevice& device,const std::vector<Vertex>& vertices) : myDevice{device}
	{
		CreateVertexBuffers(vertices);
	}

	VKBufferHandler::VKBufferHandler(MyVulkanDevice& device,const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices):myDevice{ device }
	{
		CreateVertexBuffers(vertices);
		this->indices = indices;
		CreateIndexBuffers(this->indices);
	}

	void VKBufferHandler::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(myDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(myDevice.device(), stagingBufferMemory);

		myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		myDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(myDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(myDevice.device(), stagingBufferMemory, nullptr);

		myDevice.deletionQueue.push_function([this]() {vkDestroyBuffer(myDevice.device(), vertexBuffer, nullptr);});
		myDevice.deletionQueue.push_function([this]() {vkFreeMemory(myDevice.device(), vertexBufferMemory, nullptr);});

		

	}

	void VKBufferHandler::CreateIndexBuffers(const std::vector<uint32_t>& indices)
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(myDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(myDevice.device(), stagingBufferMemory);

		myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		myDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(myDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(myDevice.device(), stagingBufferMemory, nullptr);

		myDevice.deletionQueue.push_function([this]() {vkDestroyBuffer(myDevice.device(), indexBuffer, nullptr);});
		myDevice.deletionQueue.push_function([this]() {vkFreeMemory(myDevice.device(), indexBufferMemory, nullptr);});
	}





	std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescription()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;

	}
	std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription>attributeDescription(3);
		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, position);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);


		attributeDescription[2].binding = 0;
		attributeDescription[2].location = 2;
		attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[2].offset = offsetof(Vertex, texCoord);

		return attributeDescription;

	}
}