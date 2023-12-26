#pragma once

#include <string>
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include <vector>



namespace VULKAN{

	struct PipelineConfigInfo {
	};


	class PipelineReader
	{
	public:
		PipelineReader(MyVulkanDevice& device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo configInfo);

		~PipelineReader() {};

		PipelineReader(const PipelineReader&) = delete;
		void operator=(const PipelineReader&) = delete;

		static PipelineConfigInfo DefaultPipelineDefaultConfigInfo(uint32_t width, uint32_t height);
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


