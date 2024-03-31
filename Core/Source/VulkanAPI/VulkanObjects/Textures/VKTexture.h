#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"

namespace VULKAN {


	class VKTexture : public IResource
	{
	public:

		VKTexture(const char* path, VulkanSwapChain& swapchain);
		//VKTexture(VKTexture& other);
		//Resources handled by the swapchain;
		VKTexture(VulkanSwapChain& swapchain, uint32_t width, uint32_t heigh, VkImageLayout oldLayout, VkImageLayout newLayout, VkFormat formatt);

		VKTexture operator=(VKTexture& other) {
		
			std::swap(textureImage, other.textureImage);
			std::swap(textureSampler, other.textureSampler);
			std::swap(textureImageView, other.textureImageView);
			std::swap(textureImageMemory, other.textureImageMemory);
			return other;
		
		}

		void DestroyResource() const override{
			
			vkDestroySampler(mySwapChain.device.device(), textureSampler, nullptr);
			vkDestroyImageView(mySwapChain.device.device(), textureImageView, nullptr);
			vkDestroyImage(mySwapChain.device.device(), textureImage, nullptr);
			vkFreeMemory(mySwapChain.device.device(), textureImageMemory, nullptr);
		
		}

		VkImage textureImage=nullptr;
		VkSampler textureSampler= nullptr;
		VkImageView textureImageView= nullptr;
		VkDeviceMemory textureImageMemory=nullptr;
		VkImageLayout currentLayout= VK_IMAGE_LAYOUT_UNDEFINED;
		uint32_t mipLevels=0;

	private:
		void CreateTextureImage();
		void CreateTextureSample();
		void CreateImageViews();
		void CreateStorageImage(uint32_t width, uint32_t height, VkImageLayout oldLayout, VkImageLayout newLayout, VkFormat format);
		
		VkFormat format= VK_FORMAT_R8G8B8A8_SRGB;
		VulkanSwapChain& mySwapChain;
		MyVulkanDevice& device;

		const char* path=nullptr;

	};

}