#include "Forward_RS.h"
#include "VulkanAPI/Utility/ComputeShadersUtils.h"

namespace VULKAN {


	Forward_RS::Forward_RS(VulkanRenderer& currentRenderer, MyVulkanDevice& device): renderer{ currentRenderer } , myDevice{device}
	{

	}
	Forward_RS::~Forward_RS(){

		vkDestroyPipeline(myDevice.device(), computePipeline, nullptr);
		vkDestroyPipelineLayout(myDevice.device(), computePipelineLayout, nullptr);
		vkDestroyShaderModule(myDevice.device(), computeModule, nullptr);
		vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);
	}
	void Forward_RS::CreateDescriptorSets()
	{
		renderSystemDescriptorSetHandler = std::make_unique<MyDescriptorSets>(myDevice);
		VKTexture* lion = new VKTexture("C:/Users/carlo/Downloads/VikkingRoomTextures.png", renderer.GetSwapchain());

		myDevice.TransitionImageLayout(outputStorageImage->textureImage,VK_FORMAT_R8G8B8A8_SRGB , 1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		outputStorageImage->currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		std::array <VkDescriptorSetLayoutBinding, 4> bindings;
		bindings[0] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
		bindings[1] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
		bindings[2] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);
		bindings[3] = renderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);
		renderSystemDescriptorSetHandler->CreateLayoutBinding(bindings, 1);


		renderSystemDescriptorSetHandler->CreateUniformBuffers<UniformBufferObjectData>(1, renderer.GetMaxRenderInFlight());
		renderSystemDescriptorSetHandler->CreateDescriptorPool(bindings, renderer.GetMaxRenderInFlight(), VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		renderSystemDescriptorSetHandler->CreateDescriptorSets<UniformBufferObjectData>(bindings, 1, renderer.GetMaxRenderInFlight(), *lion, *outputStorageImage,*raytracingImage,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

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
			"../Core/Source/Shaders/base_shader.vert.spv",
			"../Core/Source/Shaders/base_shader.frag.spv",
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

		storageImage = new VKTexture(renderer.GetSwapchain(), renderer.GetSwapchain().width(), renderer.GetSwapchain().height(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_FORMAT_R8G8B8A8_UNORM);
		outputStorageImage =  new VKTexture(renderer.GetSwapchain(), renderer.GetSwapchain().width(), renderer.GetSwapchain().height(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_FORMAT_R8G8B8A8_UNORM);



		computeRenderSystemDescriptorSetHandler = std::make_unique<MyDescriptorSets>(myDevice);
		std::array <VkDescriptorSetLayoutBinding,5> bindings;
		bindings[0] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);
		bindings[1] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1, 1);
		bindings[2] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2, 1);
		bindings[3] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 3, 1);
		bindings[4] = computeRenderSystemDescriptorSetHandler->CreateDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 4, 1);

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

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo.imageView = storageImage->textureImageView;
			imageInfo.sampler = storageImage->textureSampler;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = computeRenderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pImageInfo = &imageInfo;

			VkDescriptorImageInfo outputImageInfo{};
			outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			outputImageInfo.imageView = outputStorageImage->textureImageView;
			outputImageInfo.sampler = outputStorageImage->textureSampler;

			descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[4].dstSet = computeRenderSystemDescriptorSetHandler->descriptorData[0].descriptorSets[i];
			descriptorWrites[4].dstBinding = 4;
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrites[4].descriptorCount = 1;
			descriptorWrites[4].pImageInfo = &outputImageInfo;

			vkUpdateDescriptorSets(myDevice.device(),static_cast<int>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

	void Forward_RS::TransitionBeforeComputeRender(uint32_t currentImage)
	{
		if (outputStorageImage->currentLayout!=VK_IMAGE_LAYOUT_GENERAL)
		{
			myDevice.TransitionImageLayout(outputStorageImage->textureImage,VK_FORMAT_R8G8B8A8_UNORM , 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
			outputStorageImage->currentLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
	}

	void Forward_RS::TransitionBeforeForwardRender(uint32_t currentImage)
	{
		if (outputStorageImage->currentLayout!=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			myDevice.TransitionImageLayout(outputStorageImage->textureImage,VK_FORMAT_R8G8B8A8_SRGB , 1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			outputStorageImage->currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	void Forward_RS::InitForwardSystem()
	{
		//compute
		CreateUBOBuffers();
		CreateComputeDescriptorSets();
		CreateComputePipeline();


		CreateDescriptorSets();
		CreatePipelineLayout();
		CreatePipeline();
			myDevice.deletionQueue.push_function([this]()
			{
				if (computePipeline == nullptr)return;
				vkDestroyPipeline(myDevice.device(), computePipeline, nullptr);
			});
		myDevice.deletionQueue.push_function([this]()
			{
				if (computePipelineLayout == nullptr)return;
				vkDestroyPipelineLayout(myDevice.device(), computePipelineLayout, nullptr);
			});
		myDevice.deletionQueue.push_function([this]()
			{
				if (computeModule == nullptr)return;
				vkDestroyShaderModule(myDevice.device(), computeModule, nullptr);
			});
		myDevice.deletionQueue.push_function([this]()
			{
				if (pipelineLayout == nullptr)return;
				vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);
			});

	}
}

