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
		vertexBuffer.device = myDevice.device();
		indexBuffer.device = myDevice.device();
	}

	ImguiRenderSystem::~ImguiRenderSystem()
	{
	}

	void ImguiRenderSystem::CreatePipeline()
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before swapchain");
		PipelineConfigInfo pipelineConfig{};

		PipelineReader::UIPipelineDefaultConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = myRenderer.GetSwapchain().UIRenderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineConfig.multisampleInfo.rasterizationSamples = myDevice.msaaSamples;
		pipelineReader = std::make_unique<PipelineReader>(myDevice);
		pipelineReader->CreateFlexibleGraphicPipeline<UIVertex>(
			"../Core/Source/Shaders/Imgui/imgui_shader.vert.spv",
			"../Core/Source/Shaders/Imgui/imgui_shader.frag.spv",
			pipelineConfig);
	}

	void ImguiRenderSystem::CreatePipelineLayout()
	{

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBinding.pImmutableSamplers = nullptr;

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

		poolInfo.maxSets = static_cast<uint32_t>(myRenderer.GetMaxRenderInFlight()) * 2;

		if (vkCreateDescriptorPool(myDevice.device(), &poolInfo, nullptr, &imguiPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

		std::vector<VkDescriptorSetLayout> layouts(myRenderer.GetMaxRenderInFlight(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = imguiPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(myRenderer.GetMaxRenderInFlight());
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(myDevice.device(), &allocInfo, &descriptorSets) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < myRenderer.GetMaxRenderInFlight(); i++)
		{
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrite{};
			

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UIVertex);


			VkDescriptorImageInfo imageInfo{};
			
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = fontTexture->textureImageView;
			imageInfo.sampler = fontTexture->textureSampler;

			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = descriptorSets;
			descriptorWrite[0].dstBinding = 0;
			descriptorWrite[0].dstArrayElement = 0;
			descriptorWrite[0].pBufferInfo = &bufferInfo;
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite[0].descriptorCount =1;
			descriptorWrite[0].pImageInfo = &imageInfo;
			descriptorWrite[0].pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(myDevice.device(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
		}

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
	}

	void ImguiRenderSystem::InitImgui()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = 1.0f;
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(2.0f);


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
		ImGuiIO& io = ImGui::GetIO();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets, 0, nullptr);
		pipelineReader->bind(commandBuffer);

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
					//scissorRect.offset.x = 100;
					//scissorRect.offset.y = 100;
					//scissorRect.extent.width = 400;
					//scissorRect.extent.height = 400;
					vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
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

		// Init imGui windows and elements

		// Debug window
		//ImGui::SetWindowPos(ImVec2(20 * 1.0f, 20 * 1));
		//ImGui::SetWindowSize(ImVec2(300 * 1, 300 *1));
		//ImGui::TextUnformatted(myDevice.properties.deviceName);

		//SRS - ShowDemoWindow() sets its own initial position and size, cannot override here
		//ImGui::ShowDemoWindow(&show_demo_window);


	}

	void ImguiRenderSystem::EndFrame()
	{
		ImGui::Render();

	}

	void ImguiRenderSystem::SetUpSystem(GLFWwindow* window)
	{
		InitImgui();
		CreateFonts();
		SetImgui(window);


		CreatePipelineLayout();
		CreatePipeline();

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
