#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"

#include <memory>
#include <vector>
namespace VULKAN {

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
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffer();
		void DrawFrame();

		VulkanInit initWindow{ WIDTH, HEIGHT, "MyVulkanApp"};
		MyVulkanDevice myDevice{ initWindow };
		VulkanSwapChain swapChain{ myDevice, initWindow.getExtent() };
		std::unique_ptr<PipelineReader> pipelineReader;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffer;

	};

}


