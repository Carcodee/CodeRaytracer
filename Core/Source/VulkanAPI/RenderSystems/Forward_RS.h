#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DescriptorSetHandler/MyDescriptorSets.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include "VulkanAPI/Renderer/VulkanRenderer.h"


#define MAX_PARTICLES 15001 

namespace VULKAN
{
	class Forward_RS
	{
	public:
		struct UniformBufferObject {
			float deltaTime = 1.0f;
		};

		Forward_RS(VulkanRenderer& currentRender, MyVulkanDevice& device);
		~Forward_RS();
		void CreateDescriptorSets();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CheckRenderPassCompatibility();
		


		void CreateComputePipeline();
		void CreateComputeDescriptorSets();
		void CreateComputeWorkGroups(int currentFrame, VkCommandBuffer& command_buffer);
		void CreateUBOBuffers();
		void UpdateUBO(uint32_t currentImage, float deltaTime);
		void TransitionBeforeComputeRender(uint32_t currentImage);
		void TransitionBeforeForwardRender(uint32_t currentImage);
		void InitForwardSystem();
		std::unique_ptr<PipelineReader> pipelineReader;
		std::unique_ptr <MyDescriptorSets> renderSystemDescriptorSetHandler;
		std::unique_ptr <MyDescriptorSets> computeRenderSystemDescriptorSetHandler;

		VkPipelineLayout pipelineLayout;

		//compute
		std::vector<VkBuffer> shaderStorageBuffers;
		std::vector<VkDeviceMemory> shaderStorageBuffersMemory;
		std::vector<VkBuffer> uboBuffers;
		std::vector<VkDeviceMemory> uboBuffersMemory;
		std::vector<void*> uboBuffersMapped;
		VkPipelineLayout computePipelineLayout;
		VKTexture* storageImage;
		VKTexture* outputStorageImage;

		VKTexture* raytracingImage;



	private:


		MyVulkanDevice& myDevice;
		VulkanRenderer& renderer;
		VkShaderModule computeModule;
		VkPipeline computePipeline;


	};
}


