#include "MyModel.h"
#include <cassert>
#include <cstring>



namespace VULKAN {
	MyModel::MyModel(MyVulkanDevice &device, const std::vector<Vertex>& vertices): myDevice{device}  {
		
		CreateVertexBuffers(vertices);

	}
	MyModel::~MyModel()
	{
		vkDestroyBuffer(myDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(myDevice.device(), vertexBufferMemory, nullptr);

		//vkDestroySampler(myDevice.device(), textureSampler, nullptr);
		//vkDestroyImageView(myDevice.device(), textureImageView, nullptr);

		//vkDestroyImage(myDevice.device(), textureImage, nullptr);
		//vkFreeMemory(myDevice.device(), textureImageMemory, nullptr);
	}
	void MyModel::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}
	void MyModel::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void MyModel::CreateTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("Source/Resources/Assets/Images/Lion.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		myDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data; 
		vkMapMemory(myDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(myDevice.device(), stagingBufferMemory);

		stbi_image_free(pixels);

		CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
		myDevice.TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		myDevice.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		myDevice.TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(myDevice.device(), stagingBuffer, nullptr);
	    vkFreeMemory(myDevice.device(), stagingBufferMemory, nullptr);


	}
	void MyModel::CreateTextureImageView(VulkanSwapChain& swapchain)
	{
		textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
		swapchain.swapChainImageViews.resize(swapchain.swapChainImages.size());

		for (size_t i = 0; i < swapchain.swapChainImages.size(); i++)
		{
			swapchain.swapChainImageViews[i] = CreateImageView(swapchain.swapChainImages[i], swapchain.swapChainImageFormat);
		}

	}
	VkImageView MyModel::CreateImageView(VkImage image, VkFormat format)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;

		if (vkCreateImageView(myDevice.device(), &viewInfo, nullptr, &imageView)!= VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view! KEKW");
		}

		return imageView;
	}
	void MyModel::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tilling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};

		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width =width;
		imageInfo.extent.height= height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tilling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (vkCreateImage(myDevice.device(), &imageInfo, nullptr, &image)!=VK_SUCCESS);
		{
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(myDevice.device(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = myDevice.findMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(myDevice.device(), &allocInfo, nullptr, &imageMemory)!=VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}vkBindImageMemory(myDevice.device(), image, imageMemory, 0);



	}
	void MyModel::CreateTextureSample()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter= VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		//Not using anistropy;
		//samplerInfo.anisotropyEnable = VK_TRUE;
		//samplerInfo.maxAnisotropy = myDevice.properties.limits.maxSamplerAnisotropy;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;


		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod= 0.0f;
		if (vkCreateSampler(myDevice.device(), &samplerInfo, nullptr, &textureSampler)!= VK_SUCCESS);
		{
			throw std::runtime_error("Failed to create texture sampler :/ ");
		}

	}
	void MyModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}
	void MyModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(myDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(myDevice.device(), stagingBufferMemory);

	 	myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		myDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(myDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(myDevice.device(), stagingBufferMemory, nullptr);
		
	}

	std::vector<VkVertexInputBindingDescription> MyModel::Vertex::GetBindingDescription()
	{
		 std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		 bindingDescription[0].binding = 0;
		 bindingDescription[0].stride= sizeof(Vertex);
		 bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		 return bindingDescription;

	}

	std::vector<VkVertexInputAttributeDescription> MyModel::Vertex::GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription>attributeDescription(2);
		attributeDescription[0].binding = 0;
		attributeDescription[0].location= 0;
		attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, position);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		return attributeDescription;

	}
}