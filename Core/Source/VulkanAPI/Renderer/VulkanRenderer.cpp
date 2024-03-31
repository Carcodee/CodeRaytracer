#include "VulkanRenderer.h"
#include <stdexcept>
#include <array>
#include "VulkanAPI/VulkanObjects/Textures/VKTexture.h"
#include <string>
namespace VULKAN {


		VulkanRenderer::VulkanRenderer(VulkanInit& window, MyVulkanDevice& device) : initWindow{window}, myDevice{device}
		{
			CreateSwapChain();
			RecreateSwapChain();
			CreateCommandBuffer();



		}

		VulkanRenderer::~VulkanRenderer()
		{
			FreeCommandBuffers();
		}

		void VulkanRenderer::CreateCommandBuffer()
		{
			VkCommandBufferAllocateInfo allocInfo{};
		
			commandBuffer.resize(swapChain->imageCount());
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = myDevice.getCommandPool();
			allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());

			if (vkAllocateCommandBuffers(myDevice.device(), &allocInfo, commandBuffer.data()) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate command buffers!");
			}

			allocInfo={};
			computeCommandBuffers.resize(swapChain->imageCount());
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = myDevice.getCommandPool();
			allocInfo.commandBufferCount = static_cast<uint32_t>(computeCommandBuffers.size());

			if (vkAllocateCommandBuffers(myDevice.device(), &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate command buffers!");
			}
		

		}



		void VulkanRenderer::FreeCommandBuffers()
		{
			if (commandBuffer.size() > 0)
			{
				vkFreeCommandBuffers(myDevice.device(), myDevice.getCommandPool(), static_cast<uint32_t>(commandBuffer.size()), commandBuffer.data());
				commandBuffer.clear();
				computeCommandBuffers.clear();
			}

		}



		void VulkanRenderer::RecreateSwapChain()
		{
			auto extend = initWindow.getExtent();
			while (extend.width == 0 || extend.height == 0)
			{
				extend = initWindow.getExtent();
				glfwWaitEvents();
			}
			vkDeviceWaitIdle(myDevice.device());

			if (swapChain == nullptr)
			{
				swapChain = std::make_unique<VulkanSwapChain>(myDevice, extend);
			}
			else
			{
				swapChain = std::make_unique<VulkanSwapChain>(myDevice, extend, std::move(swapChain));
				if (swapChain->imageCount() != commandBuffer.size())
				{
					FreeCommandBuffers();
					CreateCommandBuffer();

				}
			}

		}

		void VulkanRenderer::CreateSwapChain()
		{
			auto extend = initWindow.getExtent();
			while (extend.width == 0 || extend.height == 0)
			{
				extend = initWindow.getExtent();
				glfwWaitEvents();
			}
			vkDeviceWaitIdle(myDevice.device());
			swapChain = std::make_unique<VulkanSwapChain>(myDevice, extend);


		}

		VkCommandBuffer VulkanRenderer::BeginFrame() {

			assert(!isFrameStarted && "Cant call beginframe while alredy in progress");

			auto result = swapChain->acquireNextImage(&currentImageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				RecreateSwapChain();
				return nullptr;
			}
			if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			{
				throw std::runtime_error("Failed to acquire swap chain image!");
			}
			isFrameStarted = true;

			auto commandBuffer = GetCurrentCommandBuffer();

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			return commandBuffer;

		}

		VkCommandBuffer VulkanRenderer::BeginComputeFrame()
		{

			swapChain->WaitForComputeFence();

			auto commandBuffer = GetCurrentComputeCommandBuffer();

			vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			return commandBuffer;
		}

		void VulkanRenderer::EndFrame() {
			assert(isFrameStarted && "Cant call beginframe while is not in progress");
			auto commandBuffer = GetCurrentCommandBuffer();

			//graphics queue
			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer!");
			}

			auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || initWindow.WasWindowResized()) {
				initWindow.ResetWindowResizedFlag();
				RecreateSwapChain();
			}
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
				initWindow.WasWindowResized()) {
				initWindow.ResetWindowResizedFlag();
				RecreateSwapChain();
			}
			else if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to present swap chain image!");
			}
			isFrameStarted = false;

		}

		void VulkanRenderer::EndComputeFrame()
		{
			//compute queue
			auto computeCommandBuffer = GetCurrentComputeCommandBuffer();


			if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer!");
			}
			auto result = swapChain->submitComputeCommandBuffers(&computeCommandBuffer, &currentImageIndex);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to present swap chain image!");
			}


		}

		void VulkanRenderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
			assert(isFrameStarted && "Cant call beginframe while is not in progress");
			assert(commandBuffer==GetCurrentCommandBuffer() && "Cant begin render pass on command buffer from a different frame");


			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = swapChain->getRenderPass();
			renderPassBeginInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);
			renderPassBeginInfo.renderArea.offset = { 0,0 };
			renderPassBeginInfo.renderArea.extent = swapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
			viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkRect2D scissor{ {0, 0 }, swapChain->getSwapChainExtent() };
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


		}
		void VulkanRenderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer) {
			assert(isFrameStarted && "Cant call beginframe while is not in progress");
			assert(commandBuffer == GetCurrentCommandBuffer() && "Cant end render pass on command buffer from a different frame");

			vkCmdEndRenderPass(commandBuffer);

		}



}