#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include "VulkanAPI/Model/MyModel.h"
#include "VulkanAPI/Utility/Utility.h"
#include <memory>
#include <vector>
#include <chrono>
#include "VulkanAPI/DescriptorSetHandler/MyDescriptorSets.h"
#include "VulkanAPI/ObjectLoader/ModelLoaderHandler.h"

#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include <functional>

namespace VULKAN {
	//
	// struct UniformBufferObjectData {
	// 	glm::mat4 model;
	// 	glm::mat4 view;
	// 	glm::mat4 projection;
	// };

	class VulkanApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void Run();
		VulkanApp();
		~VulkanApp();
		VulkanApp(const VulkanApp&) = delete;
		VulkanApp& operator=(const VulkanApp&) = delete;

#ifdef IS_EDITOR

		template <typename Lambda>
		void RunEditorFunction(Lambda myLambda) {
			myLambda();
		};

#endif

	private:
		void LoadModels();
		void CreatePipelineLayout();
		void CreatePipeline();




		int currentFrame;

		VulkanInit initWindow{ WIDTH, HEIGHT, "MyVulkanApp"};
		MyVulkanDevice myDevice{ initWindow };
		std::unique_ptr<PipelineReader> pipelineReader;
		std::unique_ptr<MyDescriptorSets> descriptorSetsHandler;
		ModelLoaderHandler* modelLoader = new ModelLoaderHandler(myDevice);
		VulkanRenderer renderer {initWindow , myDevice };
		
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<MyModel> myModel;


		

	};

}


