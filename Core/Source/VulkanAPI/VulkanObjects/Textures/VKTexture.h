#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"

namespace VULKAN {


	class VKTexture : public IResource
	{
	public:

		VKTexture(const char* path, VulkanSwapChain* swapchain, MyVulkanDevice& device);
		VKTexture(VKTexture& other)=delete;
		//Resources handled by the device;
		~VKTexture();

		VKTexture& operator=(VKTexture& other) {
			std::swap(textureImage, other.textureImage);
			std::swap(textureSampler, other.textureSampler);
			std::swap(textureImageView, other.textureImageView);
			std::swap(textureImageMemory, other.textureImageMemory);
		}

		void DestroyResource() const override{

			vkDestroySampler(myDevice.device(), textureSampler, nullptr);
			vkDestroyImageView(myDevice.device(), textureImageView, nullptr);

			vkDestroyImage(myDevice.device(), textureImage, nullptr);
			vkFreeMemory(myDevice.device(), textureImageMemory, nullptr);
		}

		VkImage textureImage;
		VkSampler textureSampler;
		VkImageView textureImageView;

	private:
		void CreateTextureImage();

		void CreateTextureSample();
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tilling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties
			, VkImage& image, VkDeviceMemory& imageMemory);
		void CreateImageViews();

		VkDeviceMemory textureImageMemory;


		MyVulkanDevice& myDevice;
		VulkanSwapChain* mySwapChain;

		const char* path;

	};

}