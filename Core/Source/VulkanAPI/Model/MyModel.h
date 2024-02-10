#pragma once
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"

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
			glm::vec2 texCoord;

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
		void CreateTextureImage();
		

		void CreateTextureSample();



		VkImageView textureImageView;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkSampler textureSampler;
	private:

		void CreateVertexBuffers(const std::vector<Vertex> &vertices);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tilling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties
			, VkImage& image, VkDeviceMemory& imageMemory);


		MyVulkanDevice &myDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

	};
}


