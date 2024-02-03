#pragma once
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include <vector>

namespace VULKAN {
	class MyModel
	{
	public:

		struct Vertex {
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();

		};
		struct triangle
		{
			std::vector<Vertex> vertices;


		};

		MyModel(MyVulkanDevice &device, const std::vector<Vertex> &vertices);
		~MyModel();
		MyModel(const MyModel&) = delete;
		MyModel& operator=(const MyModel&) = delete;

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);
		void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,VkDescriptorSet descriptorSet);

	private:
		void CreateVertexBuffers(const std::vector<Vertex> &vertices);
		MyVulkanDevice &myDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;
	};
}


