#include "VKTexture.h"


namespace VULKAN {

	VKTexture::VKTexture(const char* path, MyVulkanDevice& device) : myDevice{device}
	{
		this->path = path;
		CreateTextureImage();
		CreateImageViews();
		CreateTextureSample();
		myDevice.deletionQueue.push_function([&device, this]() {vkDestroySampler(device.device(), textureSampler, nullptr);});
		myDevice.deletionQueue.push_function([&device, this]() {vkDestroyImageView(device.device(), textureImageView, nullptr);});
		myDevice.deletionQueue.push_function([&device, this]() {vkDestroyImage(device.device(), textureImage, nullptr);});
		myDevice.deletionQueue.push_function([&device, this]() {vkFreeMemory(device.device(), textureImageMemory, nullptr);});

		//myDevice.ResourceToDestroy(this);


	}



	void VKTexture::CreateTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		myDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

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

	void VKTexture::CreateTextureSample()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
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
		samplerInfo.maxLod = 0.0f;
		if (vkCreateSampler(myDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler :/ ");
		}
	}

	void VKTexture::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tilling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};

		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
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

		if (vkCreateImage(myDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(myDevice.device(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = myDevice.findMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(myDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}vkBindImageMemory(myDevice.device(), image, imageMemory, 0);

	}

	void VKTexture::CreateImageViews()
	{

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = textureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;


		if (vkCreateImageView(myDevice.device(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view! KEKW");
		}


		
	}


}
