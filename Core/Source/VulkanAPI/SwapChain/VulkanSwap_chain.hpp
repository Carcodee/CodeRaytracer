#pragma once

#include "../DevicePipeline/Vulkan_Device.h"

// vulkan headers
#include <vulkan/vulkan.h>
// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace VULKAN {


    

class VulkanSwapChain {

public:

	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	VulkanSwapChain(MyVulkanDevice &deviceRef, VkExtent2D windowExtent);
	VulkanSwapChain(MyVulkanDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapChain> previous);

	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain &) = delete;
	VulkanSwapChain& operator=(const VulkanSwapChain &) = delete;

	VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
	VkRenderPass getRenderPass() { return renderPass; }
	VkImageView getImageView(int index) { return swapChainImageViews[index]; }
	size_t imageCount() { return swapChainImages.size(); }
	VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
	VkExtent2D getSwapChainExtent() { return swapChainExtent; }
	uint32_t width() { return swapChainExtent.width; }
	uint32_t height() { return swapChainExtent.height; }
	//change this later to a texture object
	void CreateTextureImageView(VkImageView& view, VkImage& image,uint32_t mipLevels, VkFormat format);
	void WaitForComputeFence();

	float extentAspectRatio() {
	return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
	}
	VkFormat findDepthFormat();

	VkResult acquireNextImage(uint32_t *imageIndex);
	VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
	VkResult submitComputeCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

	void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tilling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties
	, VkImage& image, VkDeviceMemory& imageMemory);
	size_t currentFrame = 0;
	void CreateImageSamples(VkSampler& sampler, float mipLevels);
	//imageviews



	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	friend class VKTexture;
	VkRenderPass UIRenderPass;
	std::vector<VkImage> colorUIImages;
	//VkDeviceMemory colorUIImagesMemory;
	std::vector<VkImageView> colorUIImageView;
	std::vector<VkFramebuffer> UIframebuffers;
//Imgui Images
	VkDeviceMemory colorImageMemory;
	VkImage colorImage;
	VkImageView colorImageView;
private:

	void Init();
	void createSwapChain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void createUIImageViews();
	void CreateUIRenderPass();
	void CreateUIFramebuffers();
	void createSyncObjects();
	void CreateColorResources();

	// Helper functions
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR> &availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

	VkImageView CreateImageView(VkImage& image, VkFormat& format, VkImageAspectFlagBits aspectFlags, uint32_t mipLevels);

	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkRenderPass renderPass;

	std::vector<VkImage> depthImages;
	std::vector<VkDeviceMemory> depthImageMemorys;
	std::vector<VkImageView> depthImageViews;




	MyVulkanDevice &device;
	VkExtent2D windowExtent;

	VkSwapchainKHR swapChain;
	std::shared_ptr<VulkanSwapChain> oldSwapChain;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	std::vector<VkSemaphore> computeRenderFinishedSemaphores;
	std::vector<VkFence> computeInFlightFences;

};

}  // namespace lve
