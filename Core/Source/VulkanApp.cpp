#include "VulkanApp.h"
#include <stdexcept>
#include <array>
namespace VULKAN{
	
	void VulkanApp::Run()
	{
		while (!initWindow.ShouldClose())
		{
			glfwPollEvents();
			DrawFrame();
		}
		vkDeviceWaitIdle(myDevice.device());
	}

	VulkanApp::VulkanApp()
	{
		LoadModels();
		CreatePipelineLayout();
		RecreateSwapChain();
		CreateCommandBuffer();
	}

	VulkanApp::~VulkanApp()
	{
		vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);

	}

	void VulkanApp::LoadModels()
	{
		std::vector<MyModel::Vertex> vertices{
			{{1.0f, 1.0f}, {1.0f, 0.0f,0.0f}},
			{{0.0f, -1.0f}, {0.0f, 1.0f,0.0f}},
			{{-1.0f, 1.0f}, {0.0f, 0.0f,1.0f}}
		};


		std::vector<MyModel::Vertex> newVertices = GetVertexPosForRecursiveTriangles(vertices, 5);


		
		myModel = std::make_unique<MyModel>(myDevice, newVertices);


	}

	void VulkanApp::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(myDevice.device(),&pipelineLayoutInfo, nullptr, &pipelineLayout)!= VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void VulkanApp::CreatePipeline()
	{
		assert(swapChain != nullptr && "Cannot create pipeline before swapchain");
		assert(pipelineLayout!= nullptr && "Cannot create pipeline before swapchain");

		PipelineConfigInfo pipelineConfig{};
		PipelineReader::DefaultPipelineDefaultConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = swapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineReader = std::make_unique<PipelineReader>(
			myDevice,
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.vert.spv",
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.frag.spv",
			pipelineConfig

		);
	}

	void VulkanApp::CreateCommandBuffer()
	{
		commandBuffer.resize(swapChain->imageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = myDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());

		if (vkAllocateCommandBuffers(myDevice.device(), &allocInfo, commandBuffer.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		 
	}

	void VulkanApp::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(myDevice.device(), myDevice.getCommandPool(),static_cast<uint32_t>(commandBuffer.size()), commandBuffer.data());
		commandBuffer.clear();
	}

	void VulkanApp::DrawFrame()
	{
		uint32_t imageIndex;
		auto result = swapChain->acquireNextImage(&imageIndex);

		if (result== VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		if (result!= VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		RecordCommandBuffer(imageIndex);
		result = swapChain->submitCommandBuffers(&commandBuffer[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || initWindow.WasWindowResized()) {
			initWindow.ResetWindowResizedFlag();
			RecreateSwapChain();
			return;
		}

		if (result!= VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}


	}

	void VulkanApp::RecreateSwapChain()
	{
		auto extend = initWindow.getExtent();
		while (extend.width == 0 || extend.height == 0)
		{
			extend = initWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(myDevice.device());

		if (swapChain==nullptr)
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

		CreatePipeline();
	}

	void VulkanApp::RecordCommandBuffer(int imageIndex)
	{

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(commandBuffer[imageIndex], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = swapChain->getRenderPass();
			renderPassBeginInfo.framebuffer = swapChain->getFrameBuffer(imageIndex);
			renderPassBeginInfo.renderArea.offset = { 0,0 };
			renderPassBeginInfo.renderArea.extent = swapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();
			
			vkCmdBeginRenderPass(commandBuffer[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
			viewport.height= static_cast<float>(swapChain->getSwapChainExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkRect2D scissor{ {0.0f}, swapChain->getSwapChainExtent() };
			vkCmdSetViewport(commandBuffer[imageIndex], 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer[imageIndex], 0, 1, &scissor);
			

			pipelineReader->bind(commandBuffer[imageIndex]);

			//vkCmdDraw(commandBuffer[imageIndex], 3, 1, 0, 0);
			myModel->Bind(commandBuffer[imageIndex]);
			myModel->Draw(commandBuffer[imageIndex]);



			vkCmdEndRenderPass(commandBuffer[imageIndex]);
			if (vkEndCommandBuffer(commandBuffer[imageIndex]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer!");
			}
		
	}


}

