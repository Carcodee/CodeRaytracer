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
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		myDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer,
			vertexBufferMemory
		);
		void* data;
		vkMapMemory(myDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(myDevice.device(), vertexBufferMemory);
	}
	std::vector<VkVertexInputBindingDescription> MyModel::Vertex::GetBindingDescription()
	{
		 std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		 bindingDescription[0].binding = 0;
		 bindingDescription[0].stride= sizeof(Vertex);
		 bindingDescription[0].binding = VK_VERTEX_INPUT_RATE_VERTEX;
		 return bindingDescription;


	}
	std::vector<VkVertexInputAttributeDescription> MyModel::Vertex::GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription>attributeDescription(1);
		attributeDescription[0].binding = 0;
		attributeDescription[0].location= 0;
		attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[0].offset = 0;
		return attributeDescription;

	}
}