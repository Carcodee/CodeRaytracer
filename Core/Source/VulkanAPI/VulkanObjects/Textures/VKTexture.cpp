#include "VKTexture.h"
#include <stb_image.h>
#include "VulkanAPI/Model/ModelHandler.h"

namespace VULKAN {

	VKTexture::VKTexture(const char* path, VulkanSwapChain& swapchain, bool addShaderId) : mySwapChain{swapchain} , device{swapchain.device}
	{

        currentStage=VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        currentAccessFlags=0;
        currentLayout= VK_IMAGE_LAYOUT_UNDEFINED;
		this->path = path;
		CreateTextureImage();
		textureImageView = mySwapChain.CreateImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT,mipLevels);
		CreateTextureSample();
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureSampler== nullptr)return;
		vkDestroySampler(device.device(), textureSampler, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureImageView== nullptr)return;
			vkDestroyImageView(device.device(), textureImageView, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureImage== nullptr)return;
			vkDestroyImage(device.device(), textureImage, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this](){
            if (textureImageMemory== nullptr)return;
			vkFreeMemory(device.device(), textureImageMemory, nullptr);
		});

		//myDevice.ResourceToDestroy(this);
        if(addShaderId){
            this->id = ModelHandler::GetInstance()->allTexturesOffset;
            ModelHandler::GetInstance()->allTexturesOnApp.try_emplace(ModelHandler::GetInstance()->allTexturesOffset,std::make_shared<VKTexture>(*this));
            ModelHandler::GetInstance()->allTexturesOffset++;
            std::cout<<" New Texture sizes: "<<ModelHandler::GetInstance()->allTexturesOffset<<"\n";
        }
	}


	VKTexture::VKTexture(VulkanSwapChain& swapchain, uint32_t width, uint32_t height, VkImageLayout newLayout,VkAccessFlags dstAccessMask,VkPipelineStageFlags stageFlags, VkFormat format, bool addShaderId) : mySwapChain{ swapchain }, device{ swapchain.device }
	{

        currentStage=VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        currentAccessFlags=0;
        currentLayout= VK_IMAGE_LAYOUT_UNDEFINED;
		CreateStorageImage(width, height, newLayout,dstAccessMask, stageFlags, format);
		CreateTextureSample();
		CreateImageViews();
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureSampler== nullptr)return;
		vkDestroySampler(device.device(), textureSampler, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureImageView== nullptr)return;
			vkDestroyImageView(device.device(), textureImageView, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureImage== nullptr)return;
			vkDestroyImage(device.device(), textureImage, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this](){
            if (textureImageMemory== nullptr)return;
			vkFreeMemory(device.device(), textureImageMemory, nullptr);
		});
        if(addShaderId){
            this->id = ModelHandler::GetInstance()->allTexturesOffset;
            ModelHandler::GetInstance()->allTexturesOnApp.try_emplace(ModelHandler::GetInstance()->allTexturesOffset,std::make_shared<VKTexture>(*this));
            ModelHandler::GetInstance()->allTexturesOffset++;
            std::cout<<" New Texture sizes: "<<ModelHandler::GetInstance()->allTexturesOffset<<"\n";
        }
    }

	VKTexture::VKTexture(VulkanSwapChain& swapchain, bool addShaderId) : mySwapChain{ swapchain }, device{ swapchain.device }
	{

        currentStage=VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        currentAccessFlags=0;
        currentLayout= VK_IMAGE_LAYOUT_UNDEFINED;
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureSampler== nullptr)return;
		vkDestroySampler(device.device(), textureSampler, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureImageView== nullptr)return;
			vkDestroyImageView(device.device(), textureImageView, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this]() {
            if (textureImage== nullptr)return;
			vkDestroyImage(device.device(), textureImage, nullptr);});
		mySwapChain.device.deletionQueue.push_function([this](){
            if (textureImageMemory== nullptr)return;
			vkFreeMemory(device.device(), textureImageMemory, nullptr);
		});
        if(addShaderId){
            this->id = ModelHandler::GetInstance()->allTexturesOffset;
            ModelHandler::GetInstance()->allTexturesOnApp.try_emplace(ModelHandler::GetInstance()->allTexturesOffset,std::make_shared<VKTexture>(*this));
            ModelHandler::GetInstance()->allTexturesOffset++;
            std::cout<<" New Texture sizes: "<<ModelHandler::GetInstance()->allTexturesOffset<<"\n";
        }
        
    }


	void VKTexture::CreateTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		mipLevels = (static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1);


		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		mySwapChain.device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mySwapChain.device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(mySwapChain.device.device(), stagingBufferMemory);

		stbi_image_free(pixels);

		mySwapChain.CreateImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			textureImage, textureImageMemory);
		mySwapChain.device.TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		mySwapChain.device.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		vkDestroyBuffer(mySwapChain.device.device(), stagingBuffer, nullptr);
		vkFreeMemory(mySwapChain.device.device(), stagingBufferMemory, nullptr);
		mySwapChain.device.GenerateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
		currentLayout= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	}

	void VKTexture::CreateImageFromSize(VkDeviceSize size,unsigned char* fontsData, uint32_t width, uint32_t height, VkFormat format)
	{

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		mipLevels = 1;
		mySwapChain.device.createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mySwapChain.device.device(), stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, fontsData, static_cast<size_t>(size));
		vkUnmapMemory(mySwapChain.device.device(), stagingBufferMemory);


		mySwapChain.CreateImage(width, height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage, textureImageMemory);
//		mySwapChain.device.TransitionImageLayout(textureImage, format, mipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        TransitionTexture(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		mySwapChain.device.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);
//		mySwapChain.device.TransitionImageLayout(textureImage, format, mipLevels, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        TransitionTexture(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		vkDestroyBuffer(mySwapChain.device.device(), stagingBuffer, nullptr);
		vkFreeMemory(mySwapChain.device.device(), stagingBufferMemory, nullptr);
//		currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	}

	void VKTexture::CreateStorageImage(uint32_t width, uint32_t height, VkImageLayout newLayout,VkAccessFlags dstAccessMask,VkPipelineStageFlags stageFlags,VkFormat format)
	{
		mySwapChain.CreateImage(width, height, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage, textureImageMemory);
            TransitionTexture(newLayout, dstAccessMask, stageFlags);
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
		//samplerInfo.maxAnisotropy = mySwapChain.device.properties.limits.maxSamplerAnisotropy;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;


		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		if (vkCreateSampler(mySwapChain.device.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler :/ ");
		}
	}

	void VKTexture::CreateImageViews(VkFormat format)
	{
		//compute image view
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

		if (vkCreateImageView(mySwapChain.device.device(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view! KEKW");
		}
	}


	void VKTexture::CreateImageViews()
	{
		//compute image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = textureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;


		if (vkCreateImageView(mySwapChain.device.device(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view! KEKW");
		}
		
	}

    VKTexture::~VKTexture() {
        if (this== nullptr){
            free(this);
        }
    }

    void VKTexture::TransitionTexture( VkImageLayout newLayout, VkAccessFlags dstAccessFlags,VkPipelineStageFlags dstStage, VkCommandBuffer& commandBuffer) {
        VkImageMemoryBarrier barrier{};

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = currentLayout;
        barrier.newLayout = newLayout;
        
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = textureImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.srcAccessMask = currentAccessFlags;
        barrier.dstAccessMask = dstAccessFlags;

        vkCmdPipelineBarrier(
                commandBuffer,
                currentStage, 
                dstStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        
        currentLayout = newLayout;
        currentStage =dstStage; 
        currentAccessFlags =dstAccessFlags;
    }


    void VKTexture::TransitionTexture(VkImageLayout newLayout, VkAccessFlags dstAccessFlags, VkPipelineStageFlags dstStage) {
        VkImageMemoryBarrier barrier{};
        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = currentLayout;
        barrier.newLayout = newLayout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = textureImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.srcAccessMask = currentAccessFlags;
        barrier.dstAccessMask = dstAccessFlags;

        vkCmdPipelineBarrier(
                commandBuffer,
                currentStage,
                dstStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

        device.endSingleTimeCommands(commandBuffer);
        currentLayout = newLayout;
        currentStage =dstStage;
        currentAccessFlags =dstAccessFlags;
    }


}
