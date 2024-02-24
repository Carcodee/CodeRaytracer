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
#include "VulkanAPI/RenderSystems/Forward_RS.h"
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
		VulkanInit initWindow{ WIDTH, HEIGHT, "MyVulkanApp" };


//#ifdef IS_EDITOR

		void RunEngine_EDITOR(std::function<void()>&& editorContext);

//
//#endif
		MyVulkanDevice myDevice{ initWindow };
		ModelLoaderHandler* modelLoader = new ModelLoaderHandler(myDevice);
		VulkanRenderer renderer{ initWindow , myDevice };
		Forward_RS forward_RS{renderer, myDevice};
		std::unique_ptr<MyModel> myModel;
		int currentFrame;

	private:
		void LoadModels();








		

	};


}


