#include "Forward_RS.h"


namespace VULKAN {


	Forward_RS::Forward_RS(VulkanRenderer& currentRenderer, MyVulkanDevice& device): renderer{ currentRenderer } , myDevice{device}
	{
		CreateDescriptorSets();
		CreatePipelineLayout();
		CreatePipeline();

	}
	Forward_RS::~Forward_RS(){

		vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);
	}
	void Forward_RS::CreateDescriptorSets()
	{
		renderSystemDescriptorSetHandler = std::make_unique<MyDescriptorSets>(myDevice);
		VKTexture* lion = new VKTexture("C:/Users/carlo/Downloads/VikkingRoomTextures.png", myDevice);
		std::array <VkDescriptorSetLayoutBinding, 2> bindings;
		bindings[0] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
		bindings[1] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
		renderSystemDescriptorSetHandler->CreateLayoutBinding(bindings, 1);
		renderSystemDescriptorSetHandler->CreateUniformBuffers<UniformBufferObjectData>(1, renderer.GetMaxRenderInFlight());
		renderSystemDescriptorSetHandler->CreateDescriptorPool(bindings, renderer.GetMaxRenderInFlight());
		renderSystemDescriptorSetHandler->CreateDescriptorSets<UniformBufferObjectData>(bindings, 1, renderer.GetMaxRenderInFlight(), *lion);


	}
	void Forward_RS::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &renderSystemDescriptorSetHandler->descriptorSetLayout[0];
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}
	void Forward_RS::CreatePipeline()
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before swapchain");

		PipelineConfigInfo pipelineConfig{};
		PipelineReader::DefaultPipelineDefaultConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderer.GetSwapchainRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;

		pipelineReader = std::make_unique<PipelineReader>(
			myDevice,
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.vert.spv",
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.frag.spv",
			pipelineConfig

		);
	}

	void Forward_RS::CheckRenderPassCompatibility()
	{

	}

}

