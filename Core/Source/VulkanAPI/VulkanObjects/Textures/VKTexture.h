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
		VKTexture(const char* path, VulkanSwapChain& swapchain, uint32_t mipLevels);
		VKTexture(VulkanSwapChain& swapchain);

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
		uint32_t mipLevels=0;

	private:
		void CreateTextureImage();
		void CreateTextureSample();
		void CreateImageViews();

		
		VkFormat format= VK_FORMAT_R8G8B8A8_SRGB;
		VulkanSwapChain& mySwapChain;
		MyVulkanDevice& device;

		const char* path=nullptr;

	};

}