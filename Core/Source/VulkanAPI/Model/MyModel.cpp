#include "MyModel.h"
#include <cassert>
#include <cstring>



namespace VULKAN {
	MyModel::MyModel(MyVulkanDevice &device, const std::vector<Vertex>& vertices): myDevice{device}  {
		
		CreateVertexBuffers(vertices);

	}
	MyModel::~MyModel()
	{
		vkDestroyBuffer(myDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(myDevice.device(), vertexBufferMemory, nullptr);
	}
	void MyModel::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}
	void MyModel::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	}
	void MyModel::CreateTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		myDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data; 
		vkMapMemory(myDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(myDevice.device(), stagingBufferMemory);

		stbi_image_free(pixels);

	}
	void MyModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}
	void MyModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
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
	}

	std::vector<VkVertexInputBindingDescription> MyModel::Vertex::GetBindingDescription()
	{
		 std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		 bindingDescription[0].binding = 0;
		 bindingDescription[0].stride= sizeof(Vertex);
		 bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		 return bindingDescription;

	}

	std::vector<VkVertexInputAttributeDescription> MyModel::Vertex::GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription>attributeDescription(2);
		attributeDescription[0].binding = 0;
		attributeDescription[0].location= 0;
		attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, position);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		return attributeDescription;

	}
}