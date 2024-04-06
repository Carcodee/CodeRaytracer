#pragma once

#include <string>
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/Model/MyModel.h"
#include <vector>



namespace VULKAN{

	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};


	class PipelineReader
	{
	public:
		PipelineReader(MyVulkanDevice& device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);
		PipelineReader(MyVulkanDevice& device);
		~PipelineReader();

		PipelineReader(const PipelineReader&) = delete;
		PipelineReader& operator=(const PipelineReader&) = delete;

		static void DefaultPipelineDefaultConfigInfo(PipelineConfigInfo& configInfo);
		static void UIPipelineDefaultConfigInfo(PipelineConfigInfo& configInfo);
		void bind(VkCommandBuffer commandBuffer);

		//RT//////////////////////////
		static VkPipelineShaderStageCreateInfo CreateShaderStageModule(VkShaderModule& module, MyVulkanDevice& device ,VkShaderStageFlagBits usage, const std::string& shaderPath);
		static VkPipelineShaderStageCreateInfo CreateComputeStageModule(VkShaderModule& module, MyVulkanDevice& device, const std::string& shaderPath);

		template <typename VertexData>
		void CreateFlexibleGraphicPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
	
	
	private:

		static std::vector <char> ReadFile(const std::string& filepath);

		void CreateGraphicPipeline(const std::string& vertFilepath,const std::string& fragFilepath, const PipelineConfigInfo& configInfo);

			 
		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
		static void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule, MyVulkanDevice& device);


		MyVulkanDevice& myVulkanDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule fragShaderModule;
		VkShaderModule vertShaderModule;

		// Pipeline cache
	    VkPipelineCache pipelineCache;
	};

	template <typename VertexData>
	void PipelineReader::CreateFlexibleGraphicPipeline(const std::string& vertFilepath, const std::string& fragFilepath,
		const PipelineConfigInfo& configInfo)
	{


		// Pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkCreatePipelineCache(myVulkanDevice.device(), &pipelineCacheCreateInfo, nullptr, &pipelineCache);
		auto vertCode = ReadFile(vertFilepath);
		auto fragcode = ReadFile(fragFilepath);

		CreateShaderModule(vertCode, &vertShaderModule);
		CreateShaderModule(fragcode, &fragShaderModule);

		assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
			"Cannor create graphics pipeline:: no pipelineLayout provided in configInfo");

		assert(configInfo.renderPass != VK_NULL_HANDLE &&
			"Cannor create graphics pipeline:: no pipelineLayout provided in configInfo");

		VkPipelineShaderStageCreateInfo shaderStage[2];

		shaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStage[0].module = vertShaderModule;
		shaderStage[0].pName = "main";
		shaderStage[0].flags = 0;
		shaderStage[0].pNext = nullptr;
		shaderStage[0].pSpecializationInfo = nullptr;

		shaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStage[1].module = fragShaderModule;
		shaderStage[1].pName = "main";
		shaderStage[1].flags = 0;
		shaderStage[1].pNext = nullptr;
		shaderStage[1].pSpecializationInfo = nullptr;


		auto bindingDescription = VertexData::GetBindingDescription();
		auto attributeDescription = VertexData::GetAttributeDescription();


		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();



		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStage;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(myVulkanDevice.device(), pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline");
		}
	}
}


