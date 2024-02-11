#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"

namespace VULKAN {


	class VKTexture : public IResource
	{
	public:

		VKTexture(const char* path, MyVulkanDevice& device);
		//VKTexture(VKTexture& other);
		//Resources handled by the device;

		VKTexture operator=(VKTexture& other) {
			std::swap(textureImage, other.textureImage);
			std::swap(textureSampler, other.textureSampler);
			std::swap(textureImageView, other.textureImageView);
			std::swap(textureImageMemory, other.textureImageMemory);
			return other;
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

		VkFormat format= VK_FORMAT_R8G8B8A8_SRGB;
		MyVulkanDevice& myDevice;

		const char* path;

	};

}