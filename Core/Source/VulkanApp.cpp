
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
			int currentTime = glfwGetTime();



			if (auto commandBuffer = renderer.BeginComputeFrame())
			{
				forward_RS.CreateComputeWorkGroups(renderer.GetCurrentFrame(), commandBuffer);
				forward_RS.UpdateUBO(renderer.GetCurrentFrame(), deltaTime);
				renderer.EndComputeFrame();
			}
			if (auto commandBuffer = renderer.BeginFrame())
			{
				renderer.BeginSwapChainRenderPass(commandBuffer);
				forward_RS.pipelineReader->bind(commandBuffer);
				forward_RS.renderSystemDescriptorSetHandler->UpdateUniformBuffer<UniformBufferObjectData>(renderer.GetCurrentFrame(), 1);

				//vkCmdDraw(commandBuffer[imageIndex], 3, 1, 0, 0);
				myModel->BindVertexBufferIndexed(commandBuffer);
				myModel->BindDescriptorSet(commandBuffer, forward_RS.pipelineLayout,forward_RS.renderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[renderer.GetCurrentFrame()]);
				myModel->DrawIndexed(commandBuffer);

				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
			deltaTime = (currentTime - lastDeltaTime) * 1000.0;
			lastDeltaTime = currentTime;
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
				forward_RS.pipelineReader->bind(commandBuffer);
				forward_RS.renderSystemDescriptorSetHandler->UpdateUniformBuffer<UniformBufferObjectData>(renderer.GetCurrentFrame(), 1);

				//vkCmdDraw(commandBuffer[imageIndex], 3, 1, 0, 0);
				myModel->BindVertexBufferIndexed(commandBuffer);
				myModel->BindDescriptorSet(commandBuffer, forward_RS.pipelineLayout, forward_RS.renderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[renderer.GetCurrentFrame()]);
				myModel->DrawIndexed(commandBuffer);

				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
			editorContext();

		}
	}

//#endif

	VulkanApp::VulkanApp()
	{
		LoadModels();

	}

	VulkanApp::~VulkanApp()
	{

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

	


}

