#pragma once
#include <memory>

#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
#include "VulkanAPI/VulkanObjects/Buffers/Buffer.h"

#define MAX_FRAMEBUFFERS 100

namespace VULKAN
{
	class ImguiRenderSystem
	{
		struct ImguiImageInfo 
		{
			VkSampler sampler;
			VkImageView imageView;
			VkDescriptorSet descriptor;
            ~ImguiImageInfo() {
                
            }
		};
    protected:
        
        static ImguiRenderSystem* instance;
        ImguiRenderSystem ( VulkanRenderer* renderer, MyVulkanDevice* myDevice);
        
	public:

		struct MyPushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} myPushConstBlock;

		ImguiRenderSystem& operator=(ImguiRenderSystem& other) = delete;
		~ImguiRenderSystem();

        static ImguiRenderSystem* GetInstance(VulkanRenderer* renderer = nullptr, MyVulkanDevice* myDevice = nullptr);
        
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
		void CreateImguiImage(VkSampler& imageSampler, VkImageView& myImageView, VkDescriptorSet& descriptor);
        void HandlePushConstantRangeRS(PushConstantBlock_RS& pushConstantBlockRs);
        void HandlePushConstantRangeBloom(PushConstantBlock_Bloom& pushConstantBlockBloom);
        void AddFramebufferReference(VKTexture* texture);
        
		bool transitionImage= false;
        
        void HandleTextureCreation(VKTexture* vkTexture);
		void CreateStyles();
		void DrawFrame(VkCommandBuffer commandBuffer);
		std::unique_ptr<PipelineReader> pipelineReader;
		VkPipelineLayout pipelineLayout;
		MyVulkanDevice* myDevice;
		VulkanRenderer* myRenderer;
		VKTexture* fontTexture;
        VKTexture* viewportTexture;
        
        std::vector<VKTexture*>frameBuffers;

        
        bool show_demo_window = true;
		bool UseDynamicRendering = false;
		float RotationSpeed=1.0f;
        int currentFrameText=1.0f;
		float camPos[3] = { 0.0f, 4.0f, 0.0f };
		float modelCamPos[3] = { 1.0f, 1.0f, 1.5f };
		float lightPos[3] = { 0.0f, 0.0f, 0.0f };
		float lightCol[3] = { 1.0f, 1.0f, 1.0f };
		float lightIntensity = 1.0f;
        float roughnessAllMaterials = 1.0f;
        float reflectivityAllMaterials = 1.0f;
        float normalAllMaterials = 1.0f;
        float allMaterialsAlpha = 1.0f;
        float allMaterialsEmissive = 1.0f;
        float allMaterialsAlbedo = 1.0f;
        float metallicAllMaterials = 1.0f;
        PushConstantBlock_RS* pushConstantBlockRsRef = nullptr;
        PushConstantBlock_Bloom* pushConstantBlockBloom = nullptr;
		char modelImporterText[128];
        VKTexture* folderThumbnail = nullptr;
        VKTexture* modelThumbnail = nullptr;

	private:
        void AddTexture(VKTexture* vkTexture);
		void SetStyle(uint32_t index);
        
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;

		VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool imguiPool = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSets;
		
		ImGuiStyle vulkanStyle;
		ImGuiStyle minimalistStyle;
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

