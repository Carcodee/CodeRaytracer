#include "Forward_RS.h"
#include "VulkanAPI/Utility/ComputeShadersUtils.h"

namespace VULKAN {


	Forward_RS::Forward_RS(VulkanRenderer& currentRenderer, MyVulkanDevice& device): renderer{ currentRenderer } , myDevice{device}
	{
		CreateDescriptorSets();
		CreatePipelineLayout();
		CreatePipeline();

		//compute
		CreateUBOBuffers();
		CreateComputeDescriptorSets();
		CreateComputePipeline();


	}
	Forward_RS::~Forward_RS(){

		vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);
	}
	void Forward_RS::CreateDescriptorSets()
	{
		renderSystemDescriptorSetHandler = std::make_unique<MyDescriptorSets>(myDevice);
		VKTexture* lion = new VKTexture("C:/Users/carlo/Downloads/VikkingRoomTextures.png", renderer.GetSwapchain());
		std::array <VkDescriptorSetLayoutBinding, 2> bindings;
		bindings[0] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
		bindings[1] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
		renderSystemDescriptorSetHandler->CreateLayoutBinding(bindings, 1);


		renderSystemDescriptorSetHandler->CreateUniformBuffers<UniformBufferObjectData>(1, renderer.GetMaxRenderInFlight());
		renderSystemDescriptorSetHandler->CreateDescriptorPool(bindings, renderer.GetMaxRenderInFlight(), VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		renderSystemDescriptorSetHandler->CreateDescriptorSets<UniformBufferObjectData>(bindings, 1, renderer.GetMaxRenderInFlight(), *lion, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

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
		pipelineConfig.multisampleInfo.rasterizationSamples = myDevice.msaaSamples;

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

	void Forward_RS::CreateComputePipeline()
	{

		
		std::string path = "C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/ComputeShaders/compute.comp.spv";
		VkPipelineShaderStageCreateInfo pipelineConfigInfo = PipelineReader::CreateComputeStageModule(computeModule, myDevice, path);



		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &computeRenderSystemDescriptorSetHandler->descriptorSetLayout[0];

		if (vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline layout!");
		}

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = computePipelineLayout;
		pipelineInfo.stage = pipelineConfigInfo;

		if (vkCreateComputePipelines(myDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline!");
		}


	}

	void Forward_RS::CreateComputeDescriptorSets()
	{
		computeRenderSystemDescriptorSetHandler = std::make_unique<MyDescriptorSets>(myDevice);
		std::array <VkDescriptorSetLayoutBinding, 3> bindings;
		bindings[0] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);
		bindings[1] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1, 1);
		bindings[2] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2, 1);

		computeRenderSystemDescriptorSetHandler->CreateLayoutBinding(bindings, 1);
		computeRenderSystemDescriptorSetHandler->CreateDescriptorPool(bindings, renderer.GetMaxRenderInFlight(), VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		computeRenderSystemDescriptorSetHandler->CreateDescriptorSets(1, renderer.GetMaxRenderInFlight());

		std::vector<Particle> particles(MAX_PARTICLES);
		InitParticles(particles);

		VkDeviceSize bufferSize = sizeof(Particle) * MAX_PARTICLES;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;


		myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(myDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, particles.data(), (size_t)bufferSize);
		vkUnmapMemory(myDevice.device(), stagingBufferMemory);


		shaderStorageBuffers.resize(renderer.GetMaxRenderInFlight());
		shaderStorageBuffersMemory.resize(renderer.GetMaxRenderInFlight());

		for (uint32_t i = 0; i < renderer.GetMaxRenderInFlight(); i++)
		{


			//Storage Buffer
			myDevice.createBuffer(bufferSize,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				shaderStorageBuffers[i],
				shaderStorageBuffersMemory[i]);
			myDevice.copyBuffer(stagingBuffer, shaderStorageBuffers[i], bufferSize);

		}
		CreateUBOBuffers();


		for(uint32_t i = 0; i < renderer.GetMaxRenderInFlight(); i++)
		{
			std::array<VkWriteDescriptorSet, bindings.size()> descriptorWrites{};

			VkDescriptorBufferInfo uniformBufferInfo{};
			uniformBufferInfo.buffer = uboBuffers[i];
			uniformBufferInfo.offset = 0;
			uniformBufferInfo.range = sizeof(UniformBufferObject);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = computeRenderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

			VkDescriptorBufferInfo storageBufferInfoLastFrame{};
			storageBufferInfoLastFrame.buffer = shaderStorageBuffers[(i - 1) % renderer.GetMaxRenderInFlight()];
			storageBufferInfoLastFrame.offset = 0;
			storageBufferInfoLastFrame.range = sizeof(Particle) * MAX_PARTICLES;


			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = computeRenderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

			VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
			storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i];
			storageBufferInfoCurrentFrame.offset = 0;
			storageBufferInfoCurrentFrame.range = sizeof(Particle) * MAX_PARTICLES;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = computeRenderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

			vkUpdateDescriptorSets(myDevice.device(), 3, descriptorWrites.data(), 0, nullptr);
		}



	}

	void Forward_RS::CreateComputeWorkGroups(int currentFrame, VkCommandBuffer& commandBuffer)
	{
		VkDescriptorSet descriptorSet = computeRenderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[currentFrame];
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &descriptorSet, 0, 0);
		vkCmdDispatch(commandBuffer, MAX_PARTICLES / 256, 1, 1);

	}
	void Forward_RS::CreateUBOBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uboBuffers.resize(renderer.GetMaxRenderInFlight());
		uboBuffersMemory.resize(renderer.GetMaxRenderInFlight());
		uboBuffersMapped.resize(renderer.GetMaxRenderInFlight());
		for (size_t i = 0; i < renderer.GetMaxRenderInFlight(); i++) {
			myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboBuffers[i], uboBuffersMemory[i]);

			vkMapMemory(myDevice.device(), uboBuffersMemory[i], 0, bufferSize, 0, &uboBuffersMapped[i]);
		}

	}

	void Forward_RS::UpdateUBO(uint32_t currentImage, float deltaTime)
	{
		UniformBufferObject ubo{};
		ubo.deltaTime = deltaTime;
		memcpy(uboBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}
}

