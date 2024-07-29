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

#include "VulkanAPI/RenderSystems/ImguiRenderSystem.h"
#include "VulkanAPI/RenderSystems/RayTracing_RS.h"
#include "VulkanAPI/RenderSystems/PostProcessing_RS.h"
#include "VulkanAPI/RenderSystems/Bloom_RS.h."


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
		static constexpr int WIDTH = 1200;
		static constexpr int HEIGHT = 900;

		VulkanApp(bool DynamicRendering, bool editor);
		~VulkanApp();
		void InitConfigsCache();
	
		VulkanApp(const VulkanApp&) = delete;
		VulkanApp& operator=(const VulkanApp&) = delete;
		VulkanInit initWindow{ WIDTH, HEIGHT, "MyVulkanApp" };


//#ifdef IS_EDITOR

		void RunDynamicRendering(std::function<void()>&& editorContext);
//
//#endif
		MyVulkanDevice myDevice{ initWindow };
		VulkanRenderer renderer{ initWindow , myDevice };
		RayTracing_RS rayTracing_RS{ myDevice, renderer};
        PostProcessing_RS postProcessing_Rs{ myDevice, renderer};
        PostProcessing_RS FinalPostProcessing_Rs{ myDevice, renderer};
        Bloom_RS bloom_Rs{myDevice, renderer};
        VKTexture* finalStorageImage;
		int currentFrame;
	    
	private:
		float deltaTime = 0.0f;
		double lastDeltaTime = 0.0f;

		void SetUpImgui();
        void LoadQueryModels();

		

	};


}


