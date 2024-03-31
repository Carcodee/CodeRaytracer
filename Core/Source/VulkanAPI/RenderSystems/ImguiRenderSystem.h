#pragma once
#include <memory>

#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>


namespace VULKAN
{
	class ImguiRenderSystem
	{
	public:
		ImguiRenderSystem (MyVulkanDevice& myDevice, VulkanRenderer renderer);
		~ImguiRenderSystem();
		void CreatePipeline();
		void CreatePipelineLayout();
		std::unique_ptr<PipelineReader> pipelineReader;
		VkPipelineLayout pipelineLayout;
		MyVulkanDevice& myDevice;
		VulkanRenderer& myRenderer;

		VKTexture* fontTexture;
		std::unique_ptr<MyDescriptorSets> imguiDescriptorSetHandler;

	};
	
}

