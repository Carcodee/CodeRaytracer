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
		ImguiRenderSystem& operator=(ImguiRenderSystem& other)
		{
			this->pipelineLayout = other.pipelineLayout;
			this->transitionImage= other.transitionImage;
			this->descriptorSetLayoutBindings= std::move(other.descriptorSetLayoutBindings);
			this->descriptorSetLayout= other.descriptorSetLayout;
			this->descriptorSets= other.descriptorSets;
			this->imguiPool= other.imguiPool;
			this->uniformBuffers= std::move(other.uniformBuffers);
			this->uniformBuffersMemory= std::move(other.uniformBuffersMemory);
			this->uniformBuffersMapped= std::move(other.uniformBuffersMapped);
			this->vulkanStyle= other.vulkanStyle;
			this->vertexBuffer= std::move(other.vertexBuffer);
			this->indexBuffer= std::move(other.indexBuffer);
			this->vertexCount= other.vertexCount;
			this->indexCount= other.indexCount;
			this->myWindow= other.myWindow;
			this->vpDescriptorSetLayoutBindings= std::move(other.vpDescriptorSetLayoutBindings);
			this->viewportSampler= other.viewportSampler;
			this->vpDescriptorSet= other.vpDescriptorSet;
			this->vpDescriptorSetLayout= other.vpDescriptorSetLayout;
			this->vpImguiPool= other.vpImguiPool;
			this->pipelineReader= std::move(other.pipelineReader);
			return *this;
		}
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
		void CreateImguiImage(VkSampler imageSampler, VkImageView myImageView);
		void DeleteImages();
		bool transitionImage= false;


		void DrawFrame(VkCommandBuffer commandBuffer);
		std::unique_ptr<PipelineReader> pipelineReader;
		VkPipelineLayout pipelineLayout;
		MyVulkanDevice& myDevice;
		VulkanRenderer& myRenderer;

		VKTexture* fontTexture;

		bool show_demo_window = true;
		float RotationSpeed=1.0f;
	private:
		void SetStyle(uint32_t index);

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		VkDescriptorSetLayout descriptorSetLayout;
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

		VkImage vpImage;
		VkImageView vpImageView;
		VkSampler viewportSampler;
		

		//Viewport
		std::vector<VkDescriptorSetLayoutBinding> vpDescriptorSetLayoutBindings;
		VkDescriptorSet vpDescriptorSet;
		VkDescriptorSetLayout vpDescriptorSetLayout;
		VkDescriptorPool vpImguiPool;
		
	};


	
}

