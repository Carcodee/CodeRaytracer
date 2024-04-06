#pragma once
#include <memory>

#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
//#include <WinUser.h>

#include "VulkanAPI/VulkanObjects/Buffers/Buffer.h"


namespace VULKAN
{
	class ImguiRenderSystem
	{
	public:
		struct MyPushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} myPushConstBlock;

		ImguiRenderSystem ( VulkanRenderer& renderer, MyVulkanDevice& myDevice);
		~ImguiRenderSystem();
		void CreatePipeline();
		void CreatePipelineLayout();
		void InitImgui();
		void SetImgui(GLFWwindow* window);
		void CreateFonts();
		void UpdateBuffers();
		void BeginFrame();
		void EndFrame();
		void SetUpSystem(GLFWwindow* window);
		void CreateImguiImage(VkSampler imageSampler, VkImageView myImageView);


		void DrawFrame(VkCommandBuffer commandBuffer);
		std::unique_ptr<PipelineReader> pipelineReader;
		VkPipelineLayout pipelineLayout;
		MyVulkanDevice& myDevice;
		VulkanRenderer& myRenderer;

		VKTexture* fontTexture;
		std::unique_ptr<MyDescriptorSets> imguiDescriptorSetHandler;

		bool show_demo_window = true;
	private:
		void SetStyle(uint32_t index);

		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		VkDescriptorSet descriptorSets;
		VkDescriptorPool imguiPool;
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;
		ImGuiStyle vulkanStyle;
		Buffer vertexBuffer;
		Buffer indexBuffer;
		int32_t vertexCount = 0;
		int32_t indexCount = 0;
		//Viewport
		VkSampler viewportSampler;
		VkDescriptorSet vpDescriptorSet;
		std::vector<VkDescriptorSetLayoutBinding> vpDescriptorSetLayoutBindings;
		VkDescriptorSetLayout vpDescriptorSetLayout;
		VkDescriptorPool vpImguiPool;
	};


	
}

