#include "PipelineReader.h"
#include <fstream>
#include <stdexcept>
#include <iostream>


namespace VULKAN
{
	PipelineReader::PipelineReader(	
		MyVulkanDevice& device,
		const std::string& vertFilepath, 
		const std::string& fragFilepath, 
		const PipelineConfigInfo configInfo) : myVulkanDevice{ device }{

		CreateGraphicPipeline(vertFilepath, fragFilepath, configInfo);	
	}

	PipelineConfigInfo PipelineReader::DefaultPipelineDefaultConfigInfo(uint32_t width, uint32_t height)
	{
		PipelineConfigInfo configInfo{};

		return configInfo;
	}

	std::vector<char> PipelineReader::ReadFile(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate, std::ios::binary);
		
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file: " + filepath);
		}
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);
		
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void PipelineReader::CreateGraphicPipeline(const std::string& vertFilepath,const std::string& fragFilepath, const PipelineConfigInfo configInfo)
	{
		auto vertCode = ReadFile(vertFilepath);
		auto fragcode = ReadFile(fragFilepath);


		std::cout << "VertSize: " << vertCode.size() << "\n";

		std::cout << "FragSize: " << fragcode.size() << "\n";
	}

	void PipelineReader::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*> (code.data());

		if (vkCreateShaderModule(myVulkanDevice.device(), &createInfo, nullptr, shaderModule) !=VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}
	}
}
