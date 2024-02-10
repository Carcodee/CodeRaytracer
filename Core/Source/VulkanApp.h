#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"
#include "VulkanAPI/Model/MyModel.h"
#include "VulkanAPI/Utility/Utility.h"
#include <memory>
#include <vector>
#include <chrono>
#include "VulkanAPI/DescriptorSetHandler/MyDescriptorSets.h"

namespace VULKAN {
	//
	// struct UniformBufferObjectData {
	// 	glm::mat4 model;
	// 	glm::mat4 view;
	// 	glm::mat4 projection;
	// };

	class VulkanApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void Run();
		VulkanApp();
		~VulkanApp();
		VulkanApp(const VulkanApp&) = delete;
		VulkanApp& operator=(const VulkanApp&) = delete;

	private:
		void LoadModels();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffer();
		void FreeCommandBuffers();
		void DrawFrame();
		void RecreateSwapChain();
		void RecordCommandBuffer(int imageIndex);
		void CreateSwapChain();
		
		void SetLayoutSetInfo();
		void CreateUniformBuffers();
		void updateUniformBuffer(uint32_t currentImage);
		void CreateDescriptorPool();
		void CreateDescriptorSets();

		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		int currentFrame;

		VulkanInit initWindow{ WIDTH, HEIGHT, "MyVulkanApp"};
		MyVulkanDevice myDevice{ initWindow };
		std::unique_ptr<PipelineReader> pipelineReader;
		std::unique_ptr<VulkanSwapChain> swapChain;

		std::unique_ptr<MyDescriptorSets> descriptorSetsHandler;
		
		VkPipelineLayout pipelineLayout;
		


		std::vector<VkCommandBuffer> commandBuffer;
		std::unique_ptr<MyModel> myModel;


	};

}


