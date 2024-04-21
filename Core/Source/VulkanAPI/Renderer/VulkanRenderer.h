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

		enum PipelineType
		{
			GRAPHICS = 0,
			COMPUTE = 1
		};

		class VulkanRenderer
		{
		public:

			VulkanRenderer(VulkanInit& window, MyVulkanDevice& device);
			~VulkanRenderer();
			VulkanRenderer(const VulkanRenderer&) = delete;
			VulkanRenderer& operator=(const VulkanRenderer&) = delete;

			VkCommandBuffer BeginFrame();
			VkCommandBuffer BeginComputeFrame();

			void EndFrame();
			void EndComputeFrame();

			void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

			void BeginUIRenderPass(VkCommandBuffer commandBuffer);
			void EndUIRenderPass(VkCommandBuffer commandBuffer);
			VkRenderPass GetSwapchainRenderPass() const { return swapChain->getRenderPass(); }
			bool isFrameInProgress()const { return isFrameStarted; }

			VkCommandBuffer GetCurrentCommandBuffer() const {
				assert(isFrameStarted && "Cannot get the command buffer when the frame is not in progress");
				return commandBuffer[currentImageIndex];
			}
			VkCommandBuffer GetCurrentComputeCommandBuffer() const {
				return computeCommandBuffers[swapChain->currentFrame];
			}
			int GetCurrentFrame() { return swapChain->currentFrame;}
			int GetMaxRenderInFlight() { return swapChain->MAX_FRAMES_IN_FLIGHT; }
			int GetImageCount() { return swapChain->imageCount(); }
			VulkanSwapChain& GetSwapchain(){ return *swapChain;}
            

		private:
			void CreateCommandBuffer();
			void FreeCommandBuffers();
			void RecreateSwapChain();
			void CreateSwapChain();
			friend class RayTracing_RS;


			uint32_t currentImageIndex=0;
			bool isFrameStarted=false;

			VulkanInit& initWindow;
			MyVulkanDevice& myDevice;
			std::unique_ptr<VulkanSwapChain> swapChain;
			std::vector<VkCommandBuffer> commandBuffer;
			std::vector<VkCommandBuffer> computeCommandBuffers;



		};

	




}


