#include "ImguiRenderSystem.h"
#include "VulkanAPI/Utility/Utility.h"
#include <algorithm>
#include <iostream>

#include "FileSystem/FileHandler.h"
#include "VulkanAPI/Model/ModelHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"
#include "VulkanAPI/ResourcesManagers/UI/ResourcesUIHandler.h"
#include "VulkanAPI/Utility/InputSystem/InputHandler.h"


namespace VULKAN
{
    ImguiRenderSystem* ImguiRenderSystem::instance = nullptr;

    ImguiRenderSystem *ImguiRenderSystem::GetInstance(VulkanRenderer* renderer, MyVulkanDevice* myDevice) {
        if (instance== nullptr){
            instance =new ImguiRenderSystem(renderer, myDevice);
        }
        return instance;
    }   
	ImguiRenderSystem::~ImguiRenderSystem()
	{
	}

	ImguiRenderSystem::ImguiRenderSystem(VulkanRenderer* renderer, MyVulkanDevice* device ) : myRenderer(renderer) ,myDevice(device) 
	{

	}


	void ImguiRenderSystem::SetUpSystem(GLFWwindow* window)
	{
		this->myWindow= window;
		
		vertexBuffer.device = myDevice->device();
		indexBuffer.device = myDevice->device();
        frameBuffers.reserve(MAX_FRAMEBUFFERS);
        
		InitImgui();
		CreateFonts();
		SetImgui(window);

		myRenderer->GetSwapchain().CreateImageSamples(viewportSampler, 1.0f);
		vpImageView= myRenderer->GetSwapchain().colorImageView;

		CreatePipelineLayout();
		CreatePipeline();
        
        std::string folderImagePath = HELPERS::FileHandler::GetInstance()->GetEngineResourcesPath() + "\\Images\\Folder.png";
        std::string modelThumbnailPath = HELPERS::FileHandler::GetInstance()->GetEngineResourcesPath() + "\\Images\\ModelImage.png";
        folderThumbnail = new VKTexture(folderImagePath.c_str(), myRenderer->GetSwapchain(), false);
        modelThumbnail = new VKTexture(modelThumbnailPath.c_str(), myRenderer->GetSwapchain(), false);
        HandleTextureCreation(folderThumbnail);
        HandleTextureCreation(modelThumbnail);
	}



	void ImguiRenderSystem::CreatePipelineLayout()
	{

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
		descriptorSetLayoutBindings.clear();
		descriptorSetLayoutBindings.push_back(layoutBinding);

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
		layoutInfo.pBindings = descriptorSetLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(myDevice->device(), &layoutInfo, nullptr, &descriptorSetLayout)!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkDeviceSize bufferSize = sizeof(UIVertex);

		uniformBuffers.resize(myRenderer->GetMaxRenderInFlight());
		uniformBuffersMemory.resize(myRenderer->GetMaxRenderInFlight());
		uniformBuffersMapped.resize(myRenderer->GetMaxRenderInFlight());
		for (size_t j = 0; j < myRenderer->GetMaxRenderInFlight(); j++)
		{
			myDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[j], uniformBuffersMemory[j]);

			vkMapMemory(myDevice->device(), uniformBuffersMemory[j], 0, bufferSize, 0, &uniformBuffersMapped[j]);

		}

        std::vector<VkDescriptorPoolSize> poolSize =
                {
                        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };
        
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = 1000;
        
		vkCreateDescriptorPool(myDevice->device(), &poolInfo, nullptr, &imguiPool);

		std::vector<VkDescriptorSetLayout> layouts(myRenderer->GetMaxRenderInFlight(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = imguiPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(myRenderer->GetMaxRenderInFlight());
		allocInfo.pSetLayouts = layouts.data();
		if (vkAllocateDescriptorSets(myDevice->device(), &allocInfo, &descriptorSets) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

        VkWriteDescriptorSet descriptorWrite{};

        VkDescriptorImageInfo imageInfo{};

        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = fontTexture->textureImageView;
        imageInfo.sampler = fontTexture->textureSampler;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount =1;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(myDevice->device(), 1, &descriptorWrite, 0, nullptr);
        
    }
	void ImguiRenderSystem::CreatePipeline()
	{
		 VkFormat format = myRenderer->GetSwapchain().getSwapChainImageFormat();
		const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats =&format,
		};
		VkPushConstantRange pushConstantRange = {};

		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // The pipeline shader stages that will access the push constant range.
		pushConstantRange.offset = 0; // The start offset, in bytes, used to access the push constants in the specified stages.
		pushConstantRange.size = sizeof(MyPushConstBlock);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(myDevice->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

		assert(pipelineLayout != nullptr && "Cannot create pipeline before swapchain");
		PipelineConfigInfo pipelineConfig{};

		PipelineReader::UIPipelineDefaultConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = myRenderer->GetSwapchain().UIRenderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineReader = std::make_unique<PipelineReader>(*myDevice);
        
        std::string shaderPath=HELPERS::FileHandler::GetInstance()->GetShadersPath();  
		pipelineReader->CreateFlexibleGraphicPipeline<UIVertex>(
			shaderPath+"/Imgui/imgui_shader.vert.spv",
            shaderPath+"/Imgui/imgui_shader.frag.spv",
			pipelineConfig, UseDynamicRendering, pipeline_rendering_create_info);
		std::cout << "You are using dynamic rendering" << "\n";
	}
	void ImguiRenderSystem::CreateImguiImage(VkSampler& imageSampler, VkImageView& myImageView, VkDescriptorSet& descriptor)
	{
        VkWriteDescriptorSet descriptorWrite{};

        VkDescriptorImageInfo imageInfo{};

        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = myImageView;
        imageInfo.sampler = imageSampler;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptor;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(myDevice->device(), 1, &descriptorWrite, 0, nullptr);

	}

	void ImguiRenderSystem::InitImgui()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = 1.0f;
		ImGuiStyle& style = ImGui::GetStyle();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		style.ScaleAllSizes(1.0f);


	}

	void ImguiRenderSystem::SetStyle(uint32_t index)
	{

		switch (index)
		{
		case 0:
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style= vulkanStyle;
			break;
		}
		case 1:
			ImGui::StyleColorsClassic();
			break;
		case 2:
			ImGui::StyleColorsDark();
			break;
		case 3:
			ImGui::StyleColorsLight();
			break;
		case 4:

			ImGuiStyle& style = ImGui::GetStyle();
			style = minimalistStyle;
			break;
		}

	}

	void ImguiRenderSystem::DisplayAllMaterialsConfigs()
	{
		ImGui::SeparatorText("All Materials Configs");
		if (ImGui::Checkbox("Use Disney BSDF", &UseDisneyBSDF))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->SetConfigVal(USE_DISNEY_BSDF, UseDisneyBSDF);
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
		}
		if (ImGui::SliderFloat("materials roughness", &roughnessAllMaterials, 0.0f, 1.0f, "%.3f"))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->materialUniform.roughnessIntensity = roughnessAllMaterials;
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
			InputHandler::editingGraphics = true;
		}
		if (ImGui::SliderFloat("materials Reflectivity", &reflectivityAllMaterials, 0.0f, 1.0f, "%.3f"))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->materialUniform.reflectivityIntensity = reflectivityAllMaterials;
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
			InputHandler::editingGraphics = true;
		}
		if (ImGui::SliderFloat("materials Normal Intensity", &normalAllMaterials, 0.0f, 2.0f, "%.3f"))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->materialUniform.normalIntensity = normalAllMaterials;
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
			InputHandler::editingGraphics = true;
		}
		if (ImGui::SliderFloat("materials Alpha Intensity", &allMaterialsAlpha, 0.0f, 1.0f, "%.3f"))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->materialUniform.alphaCutoff = allMaterialsAlpha;
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
			InputHandler::editingGraphics = true;
		}
		if (ImGui::SliderFloat("materials emissive Intensity", &allMaterialsEmissive, 0.0f, 15.0f, "%.3f"))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->materialUniform.emissionIntensity = allMaterialsEmissive;
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
			InputHandler::editingGraphics = true;
		}
		if (ImGui::SliderFloat("materials albededo Intensity", &allMaterialsAlbedo, 0.0f, 3.0f, "%.3f"))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->materialUniform.albedoIntensity = allMaterialsAlbedo;
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
			InputHandler::editingGraphics = true;
		}
		if (ImGui::SliderFloat("materials Metallic Intensity", &metallicAllMaterials, 0.0f, 1.0f, "%.3f"))
		{
			for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
				pair.second->materialUniform.metallicIntensity = metallicAllMaterials;
			}
			ModelHandler::GetInstance()->updateMaterialData = true;
			InputHandler::editingGraphics = true;
		}
    	if (UseDisneyBSDF)
	    {
		    ImGui::SeparatorText("Disney BSDF");
		    if (ImGui::Checkbox("Thin Materials", &thinMaterials))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->SetConfigVal(THIN, thinMaterials);
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
		    }
		    if (ImGui::SliderFloat("materials Anisotropic Intensity", &anisotropicAllMaterials, 0.0f, 1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.anisotropicIntensity = anisotropicAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Subsurface Intensity", &subSurfaceAllMaterials, 0.0f, 1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.subSurfaceIntensity = subSurfaceAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Clearcoat Intensity", &clearcoatAllMaterials, 0.0f, 1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.clearcoatIntensity = clearcoatAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials ClearcoatGloss Intensity", &clearcoatGlossAllMaterials, 0.0f, 1.0f,
		                           "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.clearcoatGlossIntensity = clearcoatGlossAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Refraction Intensity", &refractionAllMaterials, 1.0f, 2.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.refraction = refractionAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials relative refraction Intensity", &relativeRefractionAllMaterials, 1.0f,
		                           2.0f,
		                           "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.relativeRefraction = relativeRefractionAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials flatness intensity", &flatnessAllMaterials, 1.0f,
		                           2.0f,
		                           "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.flatness = flatnessAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Specular Intensity", &specularAllMaterials, 0.0f, 1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.specular = specularAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Specular Tint Intensity", &specularTintAllMaterials, 0.0f, 1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.specularTint = specularTintAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Sheen Intensity", &sheenAllMaterials, 0.0f, 1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.sheen = sheenAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Sheen Tint Intensity", &sheenTintAllMaterials, 0.0f, 1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.sheenTint = sheenTintAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }

		    if (ImGui::SliderFloat("materials SpecularTransmission Intensity", &specularTransmissionAllMaterials, 0.0f,
		                           1.0f, "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.specularTransmissionIntensity = specularTransmissionAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials Scatter Distance", &scatterDistance, 0.0f, 0.2f,
		                           "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.scatterDistance = scatterDistance;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::SliderFloat("materials DiffTransmission Intensity", &diffTransmissionAllMaterials, 0.0f, 1.0f,
		                           "%.3f"))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.diffTransmission = diffTransmissionAllMaterials;
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
		    if (ImGui::ColorEdit3("materials DiffTransmission Color", diffTransColAllMaterials))
		    {
			    for (auto& pair : ModelHandler::GetInstance()->allMaterialsOnApp)
			    {
				    pair.second->materialUniform.transColor = glm::make_vec3(diffTransColAllMaterials);
			    }
			    ModelHandler::GetInstance()->updateMaterialData = true;
			    InputHandler::editingGraphics = true;
		    }
	    }
		

	}

	void ImguiRenderSystem::CreateFonts()
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
		fontTexture=new VKTexture(myRenderer->GetSwapchain());
		fontTexture->CreateImageFromSize(uploadSize, fontData, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM);
		fontTexture->CreateImageViews(VK_FORMAT_R8G8B8A8_UNORM);
		fontTexture->CreateTextureSample();
        fontTexture->TransitionTexture(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}

	void ImguiRenderSystem::CreateStyles()
	{
		vulkanStyle = ImGui::GetStyle();
		vulkanStyle.FrameRounding = 3.0f;
		vulkanStyle.WindowPadding = ImVec2(8.00f, 8.00f);
		vulkanStyle.FramePadding = ImVec2(5.00f, 2.00f);
		vulkanStyle.CellPadding = ImVec2(6.00f, 6.00f);
		vulkanStyle.ItemSpacing = ImVec2(6.00f, 6.00f);
		vulkanStyle.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
		vulkanStyle.TouchExtraPadding = ImVec2(0.00f, 0.00f);
		vulkanStyle.IndentSpacing = 25;
		vulkanStyle.ScrollbarSize = 15;
		vulkanStyle.GrabMinSize = 10;
		vulkanStyle.WindowBorderSize = 1;
		vulkanStyle.ChildBorderSize = 1;
		vulkanStyle.PopupBorderSize = 1;
		vulkanStyle.FrameBorderSize = 1;
		vulkanStyle.TabBorderSize = 1;
		vulkanStyle.WindowRounding = 7;
		vulkanStyle.ChildRounding = 4;
		vulkanStyle.FrameRounding = 3;
		vulkanStyle.PopupRounding = 4;
		vulkanStyle.ScrollbarRounding = 9;
		vulkanStyle.GrabRounding = 3;
		vulkanStyle.LogSliderDeadzone = 4;
		vulkanStyle.TabRounding = 4;

		vulkanStyle.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		vulkanStyle.Colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
		vulkanStyle.Colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
		vulkanStyle.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		vulkanStyle.Colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		vulkanStyle.Colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		vulkanStyle.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		vulkanStyle.Colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		vulkanStyle.Colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		vulkanStyle.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		vulkanStyle.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		vulkanStyle.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		vulkanStyle.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		vulkanStyle.Colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		vulkanStyle.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		vulkanStyle.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		vulkanStyle.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		vulkanStyle.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		vulkanStyle.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		vulkanStyle.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		vulkanStyle.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		vulkanStyle.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		vulkanStyle.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		vulkanStyle.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);



		minimalistStyle = ImGui::GetStyle();
		minimalistStyle.Alpha = 1.0f;
		minimalistStyle.FrameRounding = 3.0f;
		minimalistStyle.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
		minimalistStyle.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		minimalistStyle.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
		minimalistStyle.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		minimalistStyle.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		minimalistStyle.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		minimalistStyle.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		minimalistStyle.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
		minimalistStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
		minimalistStyle.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		minimalistStyle.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		minimalistStyle.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		minimalistStyle.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
		minimalistStyle.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		minimalistStyle.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		minimalistStyle.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		minimalistStyle.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = minimalistStyle.Colors[i];
			float H, S, V;
			ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

			if (S < 0.1f)
			{
				V = 1.0f - V;
			}
			ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
			if (col.w < 1.00f)
			{
				col.w *= 0.3;
			}
		}


	}

	void ImguiRenderSystem::DrawFrame(VkCommandBuffer commandBuffer)
	{

		pipelineReader->bind(commandBuffer);
		ImGuiIO& io = ImGui::GetIO();
		VkViewport viewport = INITIALIZERS::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		myPushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		myPushConstBlock.translate = glm::vec2( - 1.0f);
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MyPushConstBlock), &myPushConstBlock);
		ImDrawData* imDrawData = ImGui::GetDrawData();
        
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if (imDrawData->CmdListsCount > 0) {

			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

			for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
			{
				const ImDrawList* cmd_list = imDrawData->CmdLists[i];
				for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
					VkRect2D scissorRect;
					scissorRect.offset.x =std::max((int32_t)(pcmd->ClipRect.x), 0);
					scissorRect.offset.y =std::max((int32_t)(pcmd->ClipRect.y), 0);
					scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
					scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
					vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
	
					//myDevice->TransitionImageLayout(myRenderer->GetSwapchain().colorImage, myRenderer->GetSwapchain().getSwapChainImageFormat(), 1,
					//	VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

					VkDescriptorSet currentSet= static_cast<VkDescriptorSet>(pcmd->TextureId);
					
					if (currentSet != nullptr)
					{
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &currentSet, 0, nullptr);
					}
					else
					{
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets, 0, nullptr);
					}
					vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
					indexOffset += pcmd->ElemCount;

				}
				vertexOffset += cmd_list->VtxBuffer.Size;
			}
		}
	}

	void ImguiRenderSystem::UpdateBuffers()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
			return;
		}
//        vkWaitForFences(
//                myDevice->device(),
//                1,
//                &myRenderer->GetSwapchain().inFlightFences[myRenderer->GetSwapchain().currentFrame],
//                VK_TRUE,
//                std::numeric_limits<uint64_t>::max());
//
        vkQueueWaitIdle(myDevice->graphicsQueue());
		if ((vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
			vertexBuffer.unmap();
			vertexBuffer.destroy();
			myDevice->createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.buffer, vertexBuffer.memory);
			vertexCount = imDrawData->TotalVtxCount;
			vertexBuffer.map();
		}
		if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
			indexBuffer.unmap();
			indexBuffer.destroy();
			myDevice->createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBuffer.buffer, indexBuffer.memory);
			indexCount = imDrawData->TotalIdxCount;
			indexBuffer.map();
		}
		// Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
		ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

		for (int n = 0; n < imDrawData->CmdListsCount; n++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		vertexBuffer.flush();
		indexBuffer.flush();
	}

	void ImguiRenderSystem::BeginFrame()
	{
		ImGui::NewFrame();
        
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::Begin("ViewportImage", nullptr);
		ImGui::PushID("viewport");

		ImVec2 viewportSize=ImGui::GetContentRegionAvail();

        if (viewportTexture != nullptr){
            ImGui::Image((ImTextureID)viewportTexture->textureDescriptor, ImVec2(viewportSize.x, viewportSize.y));
        }
//		ImGui::Image((ImTextureID)imagesToCreate[0]->descriptor, ImVec2(viewportSize.x, viewportSize.y));
        
        if  (ImGui::IsItemHovered()){
            InputHandler::GetInstance()->isMouseInsideViewport= true;
        }else{
            InputHandler::GetInstance()->isMouseInsideViewport= false;
            
        }

        if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_PATH"))
			{
//				const char* data = (const char*)payload->Data;
                int data = *(int*)payload->Data;

//				std::filesystem::path pathToAppend(data);
//				std::filesystem::path newPath = HELPERS::FileHandler::GetInstance()->GetAssetsPath() / pathToAppend;

//				ModelHandler::GetInstance()->AddModelToQuery(newPath.string());
				ModelHandler::GetInstance()->AddIdToQuery(data);


				ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
				ImGui::BeginTooltip();
				ImGui::Text("+");

				ImGui::EndTooltip();

			}
			ImGui::EndDragDropTarget();
		}



        ImGui::PopID();

		ResourcesUIHandler::GetInstance()->DisplayDirInfo();
        ResourcesUIHandler::GetInstance()->DisplayInspectorInfo();
        ResourcesUIHandler::GetInstance()->DisplayTexturesTab();
        ResourcesUIHandler::GetInstance()->DisplayBLASesInfo();
        ResourcesUIHandler::GetInstance()->DisplayViewportFrameBuffers(frameBuffers);
        ResourcesUIHandler::GetInstance()->DisplayEnvironments(environments, environmentSelected);
        
        
        {
            ImGui::PushID("AssetsID");
            ImGui::Begin("Configs");
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetWindowSize(ImVec2(400, 400));
            ImGui::SeparatorText("Raytracing");
            if(ImGui::Button("Add Sphere", ImVec2{50,50})){
                ModelHandler::GetInstance()->AddSphere();
                ModelHandler::GetInstance()->updateBottomLevelObj = true;
                InputHandler::editingGraphics= true;
            }
            ImGui::SeparatorText("Light");
            if(ImGui::SliderFloat3("light Pos", lightPos, -30.0f, 30.0f, "%.3f")){
                InputHandler::editingGraphics= true;
            }
            if(ImGui::ColorEdit3("light Col", lightCol, 0.0f)){
                InputHandler::editingGraphics= true;
            }
            if(ImGui::SliderFloat("light Intensity", &lightIntensity, 0.0f,15.0f,"%.3f")){
                InputHandler::editingGraphics= true;
            }

			DisplayAllMaterialsConfigs();
			
            if (pushConstantBlockRsRef != nullptr){
                ImGui::SeparatorText("Push Constants");
                HandlePushConstantRangeRS(*pushConstantBlockRsRef);
            }
            if (pushConstantBlockRsRef != nullptr){
                ImGui::SeparatorText("Bloom");
                HandlePushConstantRangeBloom(*pushConstantBlockBloom);
            }
                
            ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
            
            ImGui::End();
            ImGui::PopID();
        }
		ImGui::End(); 
        
        
        if(ModelHandler::GetInstance()->Loading){

            ImGui::PushID("Loading");
            ImGui::SeparatorText("Loading...");
            ImGui::SetWindowSize(ImVec2(400, 100));
            ImGui::SetWindowPos(ImVec2(ImGui::GetContentRegionAvail().x/2,ImGui::GetContentRegionAvail().y/2));
            ImGui::ProgressBar(sinf((float)ImGui::GetTime()) * 0.5f + 0.5f, ImVec2(ImGui::GetFontSize() * 25, 0.0f));
            ImGui::PopID();
        }

		bool open = true;
		ImGui::ShowDemoWindow(&open);

	}

	void ImguiRenderSystem::WasWindowResized()
	{
		if (myRenderer->GetSwapchain().colorImageView!=vpImageView)
		{
			SetUpSystem(myWindow);
			
		}
	}

	void ImguiRenderSystem::EndFrame()
	{
		ImGui::Render();

	}


	void ImguiRenderSystem::SetImgui(GLFWwindow* window)
	{
		// Color scheme
		CreateStyles();
		SetStyle(0);
		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		glfwGetWindowSize(window, &w, &h);
		glfwGetFramebufferSize(window, &display_w, &display_h);

		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)w, (float)h);
		io.DisplayFramebufferScale = ImVec2((float)display_w / (float)w, (float)display_h / (float)h);

		// If we directly work with os specific key codes, we need to map special key types like tab
		//io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		//io.KeyMap[ImGuiKey_Backspace] =GLFW_KEY_BACKSPACE;
		//io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		//io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		//io.KeyMap[ImGuiKey_Delete] =GLFW_KEY_DELETE;
	}

    void ImguiRenderSystem::AddTexture(VKTexture *vkTexture) {

        std::vector<VkDescriptorSetLayout> layouts(myRenderer->GetMaxRenderInFlight(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = imguiPool;

        allocInfo.descriptorSetCount = static_cast<uint32_t>(myRenderer->GetMaxRenderInFlight());
        allocInfo.pSetLayouts = layouts.data();
        if (vkAllocateDescriptorSets(myDevice->device(), &allocInfo, &vkTexture->textureDescriptor) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        CreateImguiImage(vkTexture->textureSampler , vkTexture->textureImageView, vkTexture->textureDescriptor);

    }

    void ImguiRenderSystem::HandleTextureCreation(VKTexture *vkTexture) {
        if (vkTexture->textureDescriptor == nullptr){
            AddTexture(vkTexture);
        }
    }

    void ImguiRenderSystem::HandlePushConstantRangeRS(PushConstantBlock_RS &pushConstantBlockRs) {
        if (ImGui::SliderFloat("RayTermination Bias", &pushConstantBlockRs.rayTerminationBias, 0.0f, 1.0f)){
            InputHandler::GetInstance()->editingGraphics = true;
        }
        if (ImGui::SliderFloat("Environment Intensity", &pushConstantBlockRs.environmentMapIntensity,0.0f, 2.0f)){
            InputHandler::GetInstance()->editingGraphics = true;
        }
        if (ImGui::SliderFloat("Max variance", &pushConstantBlockRs.maxVariance, 0.0f, 20.0f)){
            InputHandler::GetInstance()->editingGraphics = true;
        }
        if (ImGui::SliderInt("Min bounce for Indirect", &pushConstantBlockRs.minBounceForIndirect, 0, 5)){
            InputHandler::GetInstance()->editingGraphics = true;
        }
        if (ImGui::SliderFloat("Ambient Oclussion Intensity", &pushConstantBlockRs.AOIntensity, 0.0f, 2.0f)){
            InputHandler::GetInstance()->editingGraphics = true;
        }
        if (ImGui::SliderFloat("Ambient Oclussion Size", &pushConstantBlockRs.AOSize, 0.0f, 2.0f)){
            InputHandler::GetInstance()->editingGraphics = true;
        }




    }

    void ImguiRenderSystem::HandlePushConstantRangeBloom(PushConstantBlock_Bloom &pushConstantBlockBloom) {
        if (ImGui::SliderFloat("Bloom Size", &pushConstantBlockBloom.bloomSize, 0.0f, 10.0f)){
        }
    }

    void ImguiRenderSystem::AddFramebufferReference(VKTexture *texture) {
        frameBuffers.push_back(texture);
    }

    void ImguiRenderSystem::AddEnvReference(VKTexture* texture)
    {
    	environments.push_back(texture);
    }

    void ImguiRenderSystem::DisplayEnvironments()
    {
    	std::string path = HELPERS::FileHandler::GetInstance()->GetEngineResourcesPath();
    	
    }
}
