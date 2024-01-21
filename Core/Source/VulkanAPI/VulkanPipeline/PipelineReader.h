#pragma once

#include <string>
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/Model/MyModel.h"
#include <vector>



namespace VULKAN{

	struct PipelineConfigInfo {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
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
			const PipelineConfigInfo configInfo);

		~PipelineReader();

		PipelineReader(const PipelineReader&) = delete;
		PipelineReader& operator=(const PipelineReader&) = delete;

		static PipelineConfigInfo DefaultPipelineDefaultConfigInfo(uint32_t width, uint32_t height);

		void bind(VkCommandBuffer commandBuffer);

	private:

		static std::vector <char> ReadFile(const std::string& filepath);

		void CreateGraphicPipeline(const std::string& vertFilepath,const std::string& fragFilepath, const PipelineConfigInfo configInfo);
			 
		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		MyVulkanDevice& myVulkanDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule fragShaderModule;
		VkShaderModule vertShaderModule;

	};


}


