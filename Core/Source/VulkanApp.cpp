
#include "VulkanApp.h"
#include <stdexcept>
#include <array>
#include "VulkanAPI/VulkanObjects/Textures/VKTexture.h"
#include <string>

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
		modelLoader = new ModelLoaderHandler(myDevice);

		LoadModels();


		CreateSwapChain();
		descriptorSetsHandler = std::make_unique<MyDescriptorSets>(myDevice);


		VKTexture * lion= new VKTexture("C:/Users/carlo/Downloads/VikkingRoomTextures.png", myDevice);


		std::array <VkDescriptorSetLayoutBinding, 2> bindings;
		bindings[0] = descriptorSetsHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
		bindings[1] = descriptorSetsHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
		descriptorSetsHandler->CreateLayoutBinding(bindings,1);
		
		CreatePipelineLayout();
		RecreateSwapChain();


		descriptorSetsHandler->CreateUniformBuffers<UniformBufferObjectData>(1, swapChain->MAX_FRAMES_IN_FLIGHT);
		descriptorSetsHandler->CreateDescriptorPool(bindings, swapChain->MAX_FRAMES_IN_FLIGHT);
		descriptorSetsHandler->CreateDescriptorSets<UniformBufferObjectData>(bindings , 1, swapChain->MAX_FRAMES_IN_FLIGHT, *lion);

		CreateCommandBuffer();

	}

	VulkanApp::~VulkanApp()
	{

		vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);
	}

	void VulkanApp::LoadModels()
	{

		//const std::vector<Vertex> vertices= {
		//	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		//	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		//	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		//	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		//	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		//	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		//	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		//	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		//};
		//const std::vector<uint16_t> indices = {
		//		0, 1, 2, 2, 3, 0,
		//		4, 5, 6, 6, 7, 4
		//};
		std::string path = "C:/Users/carlo/Downloads/VikingRoom.fbx";
		VKBufferHandler* myBuffer= modelLoader->LoadModelTinyObject(path);
		
		myModel = std::make_unique<MyModel>(myDevice, *myBuffer);
		


	}

	void VulkanApp::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetsHandler->descriptorSetLayout[0];
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
		if (commandBuffer.size()>0)
		{
			vkFreeCommandBuffers(myDevice.device(), myDevice.getCommandPool(), static_cast<uint32_t>(commandBuffer.size()), commandBuffer.data());
			commandBuffer.clear();
		}

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
		descriptorSetsHandler->UpdateUniformBuffer<UniformBufferObjectData>(swapChain->currentFrame, 1);
		//updateUniformBuffer(swapChain->currentFrame);

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
			VkRect2D scissor{ {0, 0 }, swapChain->getSwapChainExtent() };
			vkCmdSetViewport(commandBuffer[imageIndex], 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer[imageIndex], 0, 1, &scissor);
			

			pipelineReader->bind(commandBuffer[imageIndex]);

			//vkCmdDraw(commandBuffer[imageIndex], 3, 1, 0, 0);
			myModel->BindVertexBufferIndexed(commandBuffer[imageIndex]);
			myModel->BindDescriptorSet(commandBuffer[imageIndex], pipelineLayout, descriptorSetsHandler->descriptorData[0].descriptorSets[swapChain->currentFrame]);

			myModel->DrawIndexed(commandBuffer[imageIndex]);



			vkCmdEndRenderPass(commandBuffer[imageIndex]);
			if (vkEndCommandBuffer(commandBuffer[imageIndex]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer!");
			}
		
	}
	void VulkanApp::CreateSwapChain()
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
	void VulkanApp::SetLayoutSetInfo()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(myDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}


	}

	void VulkanApp::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObjectData);

		uniformBuffers.resize(swapChain->MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(swapChain->MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(swapChain->MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < swapChain->MAX_FRAMES_IN_FLIGHT; i++)
		{
			myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(myDevice.device(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);

		}


	}

	void VulkanApp::updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObjectData ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.projection = glm::perspective(glm::radians(45.0f), swapChain->width() / (float)swapChain->height(), 0.1f, 10.0f);
		ubo.projection[1][1] *= -1;

		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void VulkanApp::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(swapChain->MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		poolInfo.maxSets = static_cast<uint32_t>(swapChain->MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(myDevice.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanApp::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(swapChain->MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain->MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(swapChain->MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(myDevice.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		for (size_t i = 0; i < swapChain->MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObjectData);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(myDevice.device(), 1, &descriptorWrite, 0, nullptr);
		}

	}


}

