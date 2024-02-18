#pragma once

#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"
#include "VulkanAPI/Utility/Utility.h"
#include <memory>
#include <vector>
#include <chrono>
#include "VulkanAPI/DescriptorSetHandler/MyDescriptorSets.h"
#include "VulkanAPI/ObjectLoader/ModelLoaderHandler.h"


namespace VULKAN{

		class VulkanRenderer
		{
		public:

			VulkanRenderer(VulkanInit& window, MyVulkanDevice& device);
			~VulkanRenderer();
			VulkanRenderer(const VulkanRenderer&) = delete;
			VulkanRenderer& operator=(const VulkanRenderer&) = delete;

			VkCommandBuffer BeginFrame();
			void EndFrame();
			void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);
			VkRenderPass GetSwapchainRenderPass() const { return swapChain->getRenderPass(); }
			bool isFrameInProgress()const { return isFrameStarted; }

			VkCommandBuffer GetCurrentCommandBuffer() const {
				assert(isFrameStarted && "Cannot get the command buffer when the frame is not in progress");
				return commandBuffer[currentImageIndex];
			}
			int GetCurrentFrame() { return swapChain->currentFrame;}
			int GetMaxRenderInFlight() { return swapChain->MAX_FRAMES_IN_FLIGHT; }
			int GetImageCount() { return swapChain->imageCount(); }
		private:
			void CreateCommandBuffer();
			void FreeCommandBuffers();
			void RecreateSwapChain();
			void CreateSwapChain();


			VkDescriptorSetLayout descriptorSetLayout;
			std::vector<VkBuffer> uniformBuffers;
			std::vector<VkDeviceMemory> uniformBuffersMemory;
			std::vector<void*> uniformBuffersMapped;
			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;

			uint32_t currentImageIndex;
			bool isFrameStarted=false;

			VulkanInit& initWindow;
			MyVulkanDevice& myDevice;
			std::unique_ptr<VulkanSwapChain> swapChain;
			std::vector<VkCommandBuffer> commandBuffer;


		};

	




}


