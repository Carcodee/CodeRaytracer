
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

			if (auto commandBuffer = renderer.BeginFrame())
			{
				renderer.BeginSwapChainRenderPass(commandBuffer);
				pipelineReader->bind(commandBuffer);
				descriptorSetsHandler->UpdateUniformBuffer<UniformBufferObjectData>(renderer.GetCurrentFrame(), 1);

				//vkCmdDraw(commandBuffer[imageIndex], 3, 1, 0, 0);
				myModel->BindVertexBufferIndexed(commandBuffer);
				myModel->BindDescriptorSet(commandBuffer, pipelineLayout, descriptorSetsHandler->descriptorData[0].descriptorSets[renderer.GetCurrentFrame()]);
				myModel->DrawIndexed(commandBuffer);

				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
		}
		vkDeviceWaitIdle(myDevice.device());
	}
//#ifdef IS_EDITOR

	void VulkanApp::RunEngine_EDITOR(std::function<void()>&& editorContext)
	{
		while (!initWindow.ShouldClose())
		{
			glfwPollEvents();

			if (auto commandBuffer = renderer.BeginFrame())
			{
				renderer.BeginSwapChainRenderPass(commandBuffer);
				pipelineReader->bind(commandBuffer);
				descriptorSetsHandler->UpdateUniformBuffer<UniformBufferObjectData>(renderer.GetCurrentFrame(), 1);

				//vkCmdDraw(commandBuffer[imageIndex], 3, 1, 0, 0);
				myModel->BindVertexBufferIndexed(commandBuffer);
				myModel->BindDescriptorSet(commandBuffer, pipelineLayout, descriptorSetsHandler->descriptorData[0].descriptorSets[renderer.GetCurrentFrame()]);
				myModel->DrawIndexed(commandBuffer);


				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
			//new context
			editorContext();

		}
	}

//#endif

	VulkanApp::VulkanApp()
	{
		LoadModels();
		descriptorSetsHandler = std::make_unique<MyDescriptorSets>(myDevice);
		VKTexture * lion= new VKTexture("C:/Users/carlo/Downloads/VikkingRoomTextures.png", myDevice);
		std::array <VkDescriptorSetLayoutBinding, 2> bindings;
		bindings[0] = descriptorSetsHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
		bindings[1] = descriptorSetsHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
		descriptorSetsHandler->CreateLayoutBinding(bindings,1);

		CreatePipelineLayout();
		CreatePipeline();



		descriptorSetsHandler->CreateUniformBuffers<UniformBufferObjectData>(1, renderer.GetMaxRenderInFlight());
		descriptorSetsHandler->CreateDescriptorPool(bindings, renderer.GetMaxRenderInFlight());
		descriptorSetsHandler->CreateDescriptorSets<UniformBufferObjectData>(bindings , 1, renderer.GetMaxRenderInFlight(), *lion);


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
		assert(pipelineLayout!= nullptr && "Cannot create pipeline before swapchain");

		PipelineConfigInfo pipelineConfig{};
		PipelineReader::DefaultPipelineDefaultConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderer.GetSwapchainRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;

		pipelineReader = std::make_unique<PipelineReader>(
			myDevice,
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.vert.spv",
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.frag.spv",
			pipelineConfig

		);
	}




	


}

