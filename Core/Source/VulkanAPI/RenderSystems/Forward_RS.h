#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DescriptorSetHandler/MyDescriptorSets.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include "VulkanAPI/Renderer/VulkanRenderer.h"

namespace VULKAN
{
	class Forward_RS
	{
	public:
		
		Forward_RS(VulkanRenderer& currentRender, MyVulkanDevice& device);
		~Forward_RS();
		void CreateDescriptorSets();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CheckRenderPassCompatibility();
		void CreateComputePipeline();

		std::unique_ptr<PipelineReader> pipelineReader;
		std::unique_ptr <MyDescriptorSets> renderSystemDescriptorSetHandler;
		VkPipelineLayout pipelineLayout;
		std::vector<VkBuffer> shaderStorageBuffers;
		std::vector<VkDeviceMemory> shaderStorageBuffersMemory;


	private:

		MyVulkanDevice& myDevice;
		VulkanRenderer& renderer;
		VkShaderModule computeModule;
		VkPipeline computePipeline;


	};
}


