
#include "VulkanApp.h"
#include <stdexcept>
#include <array>
#include "VulkanAPI/VulkanObjects/Textures/VKTexture.h"
#include <string>

#include "VulkanAPI/Model/ModelHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"
#include "VulkanAPI/Utility/InputSystem/InputHandler.h"
#include "FileSystem/FileHandler.h"


int cicles=0;
namespace VULKAN{
	
//#ifdef IS_EDITOR

	void VulkanApp::RunDynamicRendering(std::function<void()>&& editorContext)
	{
		while (!initWindow.ShouldClose())
		{
			glfwPollEvents();
            InputHandler::CheckMouse();
			float currentTime = glfwGetTime();
			static auto newTime = std::chrono::high_resolution_clock::now();
			auto d = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(d - newTime).count();
			rayTracing_RS.light.color = glm::make_vec3(ImguiRenderSystem::GetInstance()->lightCol);
			rayTracing_RS.light.pos = glm::make_vec3(ImguiRenderSystem::GetInstance()->lightPos);
			rayTracing_RS.light.intensity = ImguiRenderSystem::GetInstance()->lightIntensity;
            
			rayTracing_RS.cam.Move(deltaTime);
			rayTracing_RS.cam.UpdateCamera();
            if (ModelHandler::GetInstance()->updateBottomLevelObj){
                rayTracing_RS.UpdateRaytracingData();
                ModelHandler::GetInstance()->updateBottomLevelObj = false;
            }
            if (ModelHandler::GetInstance()->updateMaterialData){
                rayTracing_RS.UpdateMaterialInfo();
                ModelHandler::GetInstance()->updateMaterialData = false;
            }
            if (ModelHandler::GetInstance()->updateMeshData){
                rayTracing_RS.UpdateMeshInfo();
                ModelHandler::GetInstance()->updateMeshData = false;
            }
            if (ModelHandler::GetInstance()->updateAABBData){
                rayTracing_RS.UpdateAABBsInfo();
                ModelHandler::GetInstance()->updateAABBData= false;
            }
            VkClearValue clearValue{};
            clearValue= { 0.1f, 0.1f, 0.1f, 0.1f };
            LoadQueryModels();
            ImguiRenderSystem::GetInstance()->BeginFrame();
            ImguiRenderSystem::GetInstance()->EndFrame();
            ImguiRenderSystem::GetInstance()->UpdateBuffers();

			if (auto commandBuffer = renderer.BeginFrame())
			{
                rayTracing_RS.DrawRT(commandBuffer);
               
                rayTracing_RS.emissiveStoreImage->TransitionTexture(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, commandBuffer);
                rayTracing_RS.aoStorageImage->TransitionTexture(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, commandBuffer);

                VkMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

                vkCmdPipelineBarrier(
                        commandBuffer,
                        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0,
                        1, &barrier,
                        0, nullptr,
                        0, nullptr);

                bloom_Rs.Draw(commandBuffer);
               
                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<float>(renderer.GetSwapchain().getSwapChainExtent().width);
                viewport.height = static_cast<float>(renderer.GetSwapchain().getSwapChainExtent().height);
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                VkRect2D scissor{ {0, 0 }, renderer.GetSwapchain().getSwapChainExtent() };
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                VkRenderingInfo renderingInfoPostProc{};
                renderingInfoPostProc.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
                renderingInfoPostProc.layerCount = 1;
                renderingInfoPostProc.colorAttachmentCount = 0;
                renderingInfoPostProc.renderArea.offset = { 0, 0};
                renderingInfoPostProc.renderArea.extent = renderer.GetSwapchain().getSwapChainExtent();

                renderer.BeginDynamicRenderPass(commandBuffer,renderingInfoPostProc);
                FinalPostProcessing_Rs.Draw(commandBuffer);
                renderer.EndDynamicRenderPass(commandBuffer);          
                
                VkRenderingAttachmentInfo colorAttachmentInfo={};
                colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                colorAttachmentInfo.imageView = renderer.GetSwapchain().colorUIImageView[renderer.currentImageIndex];
                colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachmentInfo.clearValue = clearValue;

                VkRenderingInfo renderingInfo{};
                renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
                renderingInfo.layerCount = 1;
                renderingInfo.colorAttachmentCount = 1;
                renderingInfo.pColorAttachments = &colorAttachmentInfo;
                renderingInfo.renderArea.offset = { 0, 0};
                renderingInfo.renderArea.extent = renderer.GetSwapchain().getSwapChainExtent();

                finalStorageImage->TransitionTexture(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, commandBuffer);

                ImguiRenderSystem::GetInstance()->WasWindowResized();
				renderer.BeginDynamicRenderPass(commandBuffer, renderingInfo);
				ImguiRenderSystem::GetInstance()->DrawFrame(commandBuffer);
				renderer.EndDynamicRenderPass(commandBuffer);
                
				renderer.GetSwapchain().HandleColorImage(renderer.GetSwapchain().colorUIImages[renderer.currentImageIndex],
					VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    commandBuffer,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

                rayTracing_RS.emissiveStoreImage->TransitionTexture(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, commandBuffer);
                finalStorageImage->TransitionTexture(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, commandBuffer);
				renderer.EndFrame();

			}
			deltaTime = (currentTime - lastDeltaTime);
			lastDeltaTime = currentTime;
            
			AssetsHandler::GetInstance()->RegisterSaves();
			InputHandler::GetInstance()->UpdateInputStates();


		}
		vkDeviceWaitIdle(myDevice.device());

	}

	//#endif

	VulkanApp::VulkanApp(bool DynamicRendering, bool editor)
	{

        std::cout << "Present mode: currentWorkingDir: "<< std::filesystem::current_path()  << std::endl;
		rayTracing_RS.Create_RT_RenderSystem();
        rayTracing_RS.emissiveStoreImage->GenerateMipLevels();
        std::string emissiveVertPath= HELPERS::FileHandler::GetInstance()->GetShadersPath() + "\\PostPro\\postpro.vert.spv";
        std::string emissiveFragPath= HELPERS::FileHandler::GetInstance()->GetShadersPath() + "\\PostPro\\postpro.frag.spv";

		InitConfigsCache();
        ImguiRenderSystem::GetInstance(&renderer, &myDevice);
//
//        postProcessing_Rs.AddTextureImageToShader(rayTracing_RS.emissiveStoreImage->mipLevelsImagesViews[3], rayTracing_RS.emissiveStoreImage->textureSampler);
//        postProcessing_Rs.AddTextureImageToShader(rayTracing_RS.emissiveStoreImage->textureImageView, rayTracing_RS.emissiveStoreImage->textureSampler);
//        postProcessing_Rs.renderPassRef = renderer.GetSwapchain().PostProRenderPass;
//        postProcessing_Rs.InitRS(emissiveVertPath, emissiveFragPath);
//

        for (int i = 0; i < rayTracing_RS.emissiveStoreImage->mipLevelsImagesViews.size(); ++i) {
            bloom_Rs.AddTextureImageToShader(rayTracing_RS.emissiveStoreImage->mipLevelsImagesViews[i], rayTracing_RS.emissiveStoreImage->textureSampler);
        }
        bloom_Rs.upSampleRenderPassRef = renderer.GetSwapchain().UpSampleRenderPass;
        bloom_Rs.downSampleRenderPassRef = renderer.GetSwapchain().DownSampleRenderPass;
        bloom_Rs.InitRS();

        std::string outputVertPath= HELPERS::FileHandler::GetInstance()->GetShadersPath() + "\\PostPro\\postpro.vert.spv";
        std::string outputFragPath= HELPERS::FileHandler::GetInstance()->GetShadersPath() + "\\OutputShader\\outputshader.frag.spv";
//
        finalStorageImage = new VKTexture(renderer.GetSwapchain(), renderer.GetSwapchain().width(), renderer.GetSwapchain().height(),
                                                     VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_FORMAT_R8G8B8A8_UNORM, 5);
        FinalPostProcessing_Rs.AddTextureImageToShader(finalStorageImage->textureImageView, finalStorageImage->textureSampler);
        FinalPostProcessing_Rs.AddTextureImageToShader(rayTracing_RS.emissiveStoreImage->textureImageView, rayTracing_RS.emissiveStoreImage->textureSampler);
        FinalPostProcessing_Rs.AddTextureImageToShader(rayTracing_RS.aoStorageImage->textureImageView, rayTracing_RS.aoStorageImage->textureSampler);
        FinalPostProcessing_Rs.renderPassRef = renderer.GetSwapchain().FinalRenderPass; 
        FinalPostProcessing_Rs.InitRS(outputVertPath, outputFragPath);
        if (editor)
		{
			SetUpImgui();
			ImguiRenderSystem::GetInstance()->UseDynamicRendering = DynamicRendering;
			ImguiRenderSystem::GetInstance()->SetUpSystem(initWindow.window);
            ImguiRenderSystem::GetInstance()->HandleTextureCreation(finalStorageImage);
            ImguiRenderSystem::GetInstance()->viewportTexture = finalStorageImage;
            ImguiRenderSystem::GetInstance()->pushConstantBlockRsRef = &rayTracing_RS.pc;
            ImguiRenderSystem::GetInstance()->pushConstantBlockBloom = &bloom_Rs.pc;

		}
		
	}

	VulkanApp::~VulkanApp()
	{

	}

	void VulkanApp::InitConfigsCache()
	{
    
		MyVulkanDevice::g_Instance=myDevice.instance;
		MyVulkanDevice::g_PhysicalDevice=myDevice.physicalDevice;
		MyVulkanDevice::g_Device=myDevice.device();
		MyVulkanDevice::g_QueueFamily=myDevice.findPhysicalQueueFamilies().graphicsAndComputeFamily;
		MyVulkanDevice::g_Queue=myDevice.graphicsQueue_;
//		MyVulkanDevice::g_DescriptorPool=forward_RS.renderSystemDescriptorSetHandler->descriptorPool;
//		MyVulkanDevice::g_DescriptorPool=imguiDescriptorPool;
		MyVulkanDevice::g_MinImageCount=2;
		MyVulkanDevice::g_SwapChainRebuild = false;

	}


	 void VulkanApp::SetUpImgui()
        {

                // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

            // Setup Platform/Renderer backends
            bool result = ImGui_ImplGlfw_InitForVulkan(initWindow.window, true);
            if (result)
            {
                std::cout << "Imgui window init success" << "\n";
            }

        }

    void VulkanApp::LoadQueryModels() {

        if (!ModelHandler::GetInstance()->queryModelIdsToHandle.empty())
        {

            ModelHandler::GetInstance()->LoadAllModelsFromDisc();
            for (auto element :ModelHandler::GetInstance()->modelsReady)
            {
                //std::shared_ptr<ModelHandler::ModelToLoadState> current = element;
                if (element.get()->state== ModelHandler::UNLOADED || element.get()->state== ModelHandler::DISPACHED)
                {
                    continue;
                }

                rayTracing_RS.AddModelToPipeline(element->model);
                element.get()->state = ModelHandler::DISPACHED;
            }
            if (ModelHandler::GetInstance()->modelsReady.size() == ModelHandler::GetInstance()->queryModelIdsToHandle.size())
            {
                ModelHandler::GetInstance()->queryModelIdsToHandle.clear();
                ModelHandler::GetInstance()->modelsReady.clear();
                ModelHandler::GetInstance()->Loading = false;
            }
            if (rayTracing_RS.updateDescriptorData)
            {
                rayTracing_RS.UpdateRaytracingData();
                rayTracing_RS.updateDescriptorData = false;
            }

        }
        if (!ModelHandler::GetInstance()->queryModelPathsToHandle.empty())
        {
            ModelHandler::GetInstance()->LoadAllModels();
            for (auto element :ModelHandler::GetInstance()->modelsReady)
            {
                //std::shared_ptr<ModelHandler::ModelToLoadState> current = element;
                if (element.get()->state== ModelHandler::UNLOADED || element.get()->state== ModelHandler::DISPACHED)
                {
                    continue;
                }

                rayTracing_RS.AddModelToPipeline(element.get()->model);
                element.get()->state = ModelHandler::DISPACHED;
            }
            if (ModelHandler::GetInstance()->modelsReady.size() == ModelHandler::GetInstance()->queryModelPathsToHandle.size())
            {
                ModelHandler::GetInstance()->queryModelPathsToHandle.clear();
                ModelHandler::GetInstance()->modelsReady.clear();
                ModelHandler::GetInstance()->Loading = false;
            }
            if (rayTracing_RS.updateDescriptorData)
            {
                rayTracing_RS.UpdateRaytracingData();
                rayTracing_RS.updateDescriptorData = false;
            }
        }       
    }
}

