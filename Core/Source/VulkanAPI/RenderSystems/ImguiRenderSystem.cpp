#include "ImguiRenderSystem.h"
#include "VulkanAPI/Utility/Utility.h"
#include <algorithm>
#include <iostream>


namespace VULKAN
{
	ImguiRenderSystem::ImguiRenderSystem(VulkanRenderer& renderer, MyVulkanDevice& device ) : myRenderer(renderer) ,myDevice(device) 
	{

		//TODO: Fonts texture
		//TODO: Vertex buffer
		//TODO: Push constants

	}

	ImguiRenderSystem::~ImguiRenderSystem()
	{

		//vkDestroySampler(myDevice.device(),viewportSampler, nullptr);
		vkDestroyDescriptorSetLayout(myDevice.device(), descriptorSetLayout, nullptr);
		for (size_t i = 0; i < myRenderer.GetMaxRenderInFlight(); i++)
		{
			if (uniformBuffers.size()>0)
			{
				vkDestroyBuffer(myDevice.device(), uniformBuffers[i], nullptr);
				vkFreeMemory(myDevice.device(), uniformBuffersMemory[i], nullptr);
				
			}
		}
		vkDeviceWaitIdle(myDevice.device());

	}

	void ImguiRenderSystem::SetUpSystem(GLFWwindow* window)
	{
		this->myWindow= window;
		
		vertexBuffer.device = myDevice.device();
		indexBuffer.device = myDevice.device();
		InitImgui();
		CreateFonts();
		SetImgui(window);

		myRenderer.GetSwapchain().CreateImageSamples(viewportSampler, 1.0f);
		vpImageView= myRenderer.GetSwapchain().colorImageView;

		CreatePipelineLayout();
		CreateImguiImage(viewportSampler, vpImageView);
		CreatePipeline();
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

		if (vkCreateDescriptorSetLayout(myDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout)!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkDeviceSize bufferSize = sizeof(UIVertex);

		uniformBuffers.resize(myRenderer.GetMaxRenderInFlight());
		uniformBuffersMemory.resize(myRenderer.GetMaxRenderInFlight());
		uniformBuffersMapped.resize(myRenderer.GetMaxRenderInFlight());
		for (size_t j = 0; j < myRenderer.GetMaxRenderInFlight(); j++)
		{
			myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[j], uniformBuffersMemory[j]);

			vkMapMemory(myDevice.device(), uniformBuffersMemory[j], 0, bufferSize, 0, &uniformBuffersMapped[j]);

		}


		std::array<VkDescriptorPoolSize, 1> poolSize{};
		for (size_t i = 0; i < descriptorSetLayoutBindings.size(); i++)
		{
			poolSize[i].type = descriptorSetLayoutBindings[i].descriptorType;
			if (poolSize[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				poolSize[i].descriptorCount = static_cast<uint32_t>(myRenderer.GetMaxRenderInFlight()) * 2;
				continue;
			}
			poolSize[i].descriptorCount = static_cast<uint32_t>((myRenderer.GetMaxRenderInFlight()));

		}
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(myRenderer.GetMaxRenderInFlight()*2);
		vkCreateDescriptorPool(myDevice.device(), &poolInfo, nullptr, &imguiPool);

		std::vector<VkDescriptorSetLayout> layouts(myRenderer.GetMaxRenderInFlight(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = imguiPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(myRenderer.GetMaxRenderInFlight());
		allocInfo.pSetLayouts = layouts.data();
		VkDescriptorSetAllocateInfo vpAllocInfo{};
		vpAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vpAllocInfo.descriptorPool = imguiPool;
		vpAllocInfo.descriptorSetCount = static_cast<uint32_t>(myRenderer.GetMaxRenderInFlight());
		vpAllocInfo.pSetLayouts = layouts.data();
		if (vkAllocateDescriptorSets(myDevice.device(), &allocInfo, &descriptorSets) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		if (vkAllocateDescriptorSets(myDevice.device(), &vpAllocInfo, &vpDescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		for (size_t i = 0; i < myRenderer.GetMaxRenderInFlight(); i++)
		{
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrite{};
			

			VkDescriptorImageInfo imageInfo{};
			
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = fontTexture->textureImageView;
			imageInfo.sampler = fontTexture->textureSampler;

			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = descriptorSets;
			descriptorWrite[0].dstBinding = 0;
			descriptorWrite[0].dstArrayElement = 0;
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite[0].descriptorCount =1;
			descriptorWrite[0].pImageInfo = &imageInfo;
			descriptorWrite[0].pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(myDevice.device(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
		}
		


	}
	void ImguiRenderSystem::CreatePipeline()
	{
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
		if (vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

		assert(pipelineLayout != nullptr && "Cannot create pipeline before swapchain");
		PipelineConfigInfo pipelineConfig{};

		PipelineReader::UIPipelineDefaultConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = myRenderer.GetSwapchain().UIRenderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineReader = std::make_unique<PipelineReader>(myDevice);
		pipelineReader->CreateFlexibleGraphicPipeline<UIVertex>(
			"../Core/Source/Shaders/Imgui/imgui_shader.vert.spv",
			"../Core/Source/Shaders/Imgui/imgui_shader.frag.spv",
			pipelineConfig);
	}
	void ImguiRenderSystem::CreateImguiImage(VkSampler imageSampler, VkImageView myImageView)
	{


		//std::vector<VkDescriptorSetLayout> layouts(myRenderer.GetMaxRenderInFlight(), descriptorSetLayout);
		//VkDescriptorSetAllocateInfo allocInfo{};
		//allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//allocInfo.descriptorPool = imguiPool;
		//allocInfo.descriptorSetCount = static_cast<uint32_t>(myRenderer.GetMaxRenderInFlight());
		//allocInfo.pSetLayouts = layouts.data();

		//if (vkAllocateDescriptorSets(myDevice.device(), &allocInfo, &vpDescriptorSet) != VK_SUCCESS)
		//{
		//	throw std::runtime_error("failed to allocate descriptor sets!");
		//}

		for (size_t i = 0; i < myRenderer.GetMaxRenderInFlight(); i++)
		{

			std::array<VkWriteDescriptorSet, 1> descriptorWrite{};

			VkDescriptorImageInfo imageInfo{};

			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = myImageView;
			imageInfo.sampler = imageSampler;

			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = vpDescriptorSet;
			descriptorWrite[0].dstBinding = 0;
			descriptorWrite[0].dstArrayElement = 0;
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite[0].descriptorCount = 1;
			descriptorWrite[0].pImageInfo = &imageInfo;
			descriptorWrite[0].pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(myDevice.device(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
		}

	}

	void ImguiRenderSystem::DeleteImages()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();

		if (imDrawData->CmdListsCount > 0) {

			VkDeviceSize offsets[1] = { 0 };

			for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
			{
				const ImDrawList* cmd_list = imDrawData->CmdLists[i];
				for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
				{
					

				}
			}
		}
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
		}

	}

	void ImguiRenderSystem::CreateFonts()
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
		fontTexture=new VKTexture(myRenderer.GetSwapchain());
		fontTexture->CreateImageFromSize(uploadSize, fontData, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM);
		fontTexture->CreateImageViews(VK_FORMAT_R8G8B8A8_UNORM);
		fontTexture->CreateTextureSample();
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
	
					//myDevice.TransitionImageLayout(myRenderer.GetSwapchain().colorImage, myRenderer.GetSwapchain().getSwapChainImageFormat(), 1,
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
		if ((vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
			vertexBuffer.unmap();
			vertexBuffer.destroy();
			myDevice.createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.buffer, vertexBuffer.memory);

			vertexCount = imDrawData->TotalVtxCount;
			vertexBuffer.map();
		}
		if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
			indexBuffer.unmap();
			indexBuffer.destroy();
			myDevice.createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBuffer.buffer, indexBuffer.memory);
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

		// Start the Dear ImGui frame
		int width = 0;
		int height = 0;
		glfwGetWindowSize(myWindow,&width,&height);
		ImGui::SetWindowSize(ImVec2(width, height));
		ImGui::Begin("DockSpace Demo", nullptr, ImGuiWindowFlags_MenuBar  | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImVec2 viewportSize=ImGui::GetContentRegionAvail();
		ImGui::Image((ImTextureID)vpDescriptorSet, ImVec2(viewportSize.x, viewportSize.y));

		ImGui::End(); // End DockSpace Demo window
		// Make the window full-screen and set the dock space
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::SliderFloat("Speed", &RotationSpeed, 0.0f, 10.0f, "%.3f");
		
		ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		//if (ImGui::Begin("DockSpace Demo", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking))
		//{
		//	ImGui::DockSpace(ImGui::GetID("MyDockSpace"));
		//}
		//	ImGui::End(); 


	}

	void ImguiRenderSystem::WasWindowResized()
	{
		if (myRenderer.GetSwapchain().colorImageView!=vpImageView)
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
		vulkanStyle = ImGui::GetStyle();
		vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
		vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

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
		//io.KeyMap[ImGuiKey_Tab] = VK_TAB;
		//io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		//io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		//io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
		//io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
		//io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
		//io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
		//io.KeyMap[ImGuiKey_Space] = VK_SPACE;
		//io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	}

}
