#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"


namespace VULKAN {

	class VulkanApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void Run();

	private:
		VulkanInit initWindow{ WIDTH, HEIGHT, "MyVulkanApp"};
		MyVulkanDevice device{ initWindow };

		PipelineReader pipelineReader{ device,
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.vert.spv",
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.frag.spv",
			PipelineReader::DefaultPipelineDefaultConfigInfo(WIDTH, HEIGHT)};
		

	};

}


