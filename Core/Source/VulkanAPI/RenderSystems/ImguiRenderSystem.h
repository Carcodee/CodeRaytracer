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
		struct ImguiImageInfo 
		{
			VkSampler sampler;
			VkImageView imageView;
			VkDescriptorSet descriptor;
		};
	public:

		struct MyPushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} myPushConstBlock;

		ImguiRenderSystem& operator=(ImguiRenderSystem& other) = delete;

		ImguiRenderSystem ( VulkanRenderer& renderer, MyVulkanDevice& myDevice);
		~ImguiRenderSystem();
		void CreatePipeline();
		void CreatePipelineLayout();
		void InitImgui();
		void SetImgui(GLFWwindow* window);
		void CreateFonts();
		void UpdateBuffers();
		void BeginFrame();
		void WasWindowResized();
		void EndFrame();
		void SetUpSystem(GLFWwindow* window);
		void CreateImguiImage(VkSampler imageSampler, VkImageView myImageView, VkDescriptorSet& descriptor);

		bool transitionImage= false;

		void AddImage(VkSampler sampler, VkImageView image, VkDescriptorSet& descriptor);
		void AddSamplerAndViewForImage(VkSampler sampler, VkImageView view);

		void DrawFrame(VkCommandBuffer commandBuffer);
		std::unique_ptr<PipelineReader> pipelineReader;
		VkPipelineLayout pipelineLayout;
		MyVulkanDevice& myDevice;
		VulkanRenderer& myRenderer;
		VKTexture* fontTexture;

		bool show_demo_window = true;
		bool UseDynamicRendering = false;
		float RotationSpeed=1.0f;
		float camPos[3] = { 0.0f, 4.0f, 0.0f };
		float modelCamPos[3] = { 1.0f, 1.0f, 1.5f };
		float lightPos[3] = { 0.0f, 0.0f, 0.0f };
		float lightCol[3] = { 1.0f, 1.0f, 1.0f };
		float lightIntensity = 1.0f;
		char modelImporterText[128];

	private:
		void SetStyle(uint32_t index);

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		std::vector<ImguiImageInfo> imagesToCreate;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet vpDescriptorSet;
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
		GLFWwindow* myWindow;

		VkImageView vpImageView;
		VkSampler viewportSampler;
		

		//Viewport
		


	};


	
}

