#include "ImguiRenderSystem.h"

namespace VULKAN
{
	ImguiRenderSystem::ImguiRenderSystem(MyVulkanDevice& myDevice, VulkanRenderer renderer) : myDevice(myDevice), myRenderer(renderer)
	{
	}

	ImguiRenderSystem::~ImguiRenderSystem()
	{
	}

	void ImguiRenderSystem::CreatePipeline()
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before swapchain");

		PipelineConfigInfo pipelineConfig{};
		PipelineReader::DefaultPipelineDefaultConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = myRenderer.GetSwapchainRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineConfig.multisampleInfo.rasterizationSamples = myDevice.msaaSamples;
		pipelineReader = std::make_unique<PipelineReader>(
			myDevice,
			"../Core/Source/Shaders/Imgui/imgui_shader.vert.spv",
			"../Core/Source/Shaders/Imgui/imgui_shader.frag.spv",
			pipelineConfig

		);

	}

	void ImguiRenderSystem::CreatePipelineLayout()
	{


	}
}
