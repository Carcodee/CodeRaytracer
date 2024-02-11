#pragma once
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"
#include "VulkanAPI/VulkanObjects/Buffers/VKBufferHandler.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include <vector>


namespace VULKAN {
	class MyModel
	{
	public:


		MyModel(MyVulkanDevice &device, VKBufferHandler& buffer);
		~MyModel();
		MyModel(const MyModel&) = delete;
		MyModel& operator=(const MyModel&) = delete;

		void BindVertexBuffer(VkCommandBuffer commandBuffer);
		void BindVertexBufferIndexed(VkCommandBuffer commandBuffer);

		void Draw(VkCommandBuffer commandBuffer);
		void DrawIndexed(VkCommandBuffer commandBuffer);

		void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,VkDescriptorSet descriptorSet);

	private:


		VKBufferHandler buffer;
		MyVulkanDevice &myDevice;
		//VkBuffer vertexBuffer;
		//VkDeviceMemory vertexBufferMemory;

	};
}


