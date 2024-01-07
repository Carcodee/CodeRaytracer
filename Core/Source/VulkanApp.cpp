#include "VulkanApp.h"
#include <stdexcept>

namespace VULKAN{
	
	void VulkanApp::Run()
	{
		initWindow.Run();
	}

	VulkanApp::VulkanApp()
	{
		LoadModels();
		CreatePipelineLayout();
		CreatePipeline();
		CreateCommandBuffer();
	}

	VulkanApp::~VulkanApp()
	{
		vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);

	}

	void VulkanApp::LoadModels()
	{
		std::vector<MyModel::Vertex> vertices{
			{{0.0f, -0.5f}},
			{{-0.5f, -0.5f}},
			{{0.5f, 0.5f}}
		};
		myModel = std::make_unique<MyModel>(myDevice.device(), vertices);
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
		auto pipelineConfig = PipelineReader::DefaultPipelineDefaultConfigInfo(WIDTH, HEIGHT);
		pipelineConfig.renderPass = swapChain.getRenderPass();
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
	}

	void VulkanApp::DrawFrame()
	{
	}

}

