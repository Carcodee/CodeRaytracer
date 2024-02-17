#include "MyModel.h"
#include <cassert>
#include <cstring>



namespace VULKAN {
	MyModel::MyModel(MyVulkanDevice& device, VKBufferHandler& bufferObject) : myDevice{ device }, buffer{bufferObject} {
		

	}
	

	MyModel::~MyModel()
	{

	}
	void MyModel::BindVertexBuffer(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { buffer.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void MyModel::BindVertexBufferIndexed(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { buffer.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, buffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	}
	void MyModel::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, buffer.vertexCount, 1, 0, 0);
	}

	void MyModel::DrawIndexed(VkCommandBuffer commandBuffer)
	{
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(buffer.indices.size()), 1, 0, 0, 0);
	}

	void MyModel::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	}


}