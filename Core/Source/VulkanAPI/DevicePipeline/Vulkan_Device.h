#pragma once

#include "VulkanAPI/VulkanInit/VulkanInit.h"
// std lib headers
#include <string>
#include <vector>
#include "VulkanAPI/VulkanObjects/ResourceInterface/IResource.h"
#include <stdlib.h>
static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}
namespace VULKAN {


struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  uint32_t graphicsAndComputeFamily;
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class MyVulkanDevice {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  MyVulkanDevice(VulkanInit& window);
  ~MyVulkanDevice();

  // Not copyable or movable

	inline static VkAllocationCallbacks*   g_Allocator = nullptr;
	inline static VkInstance               g_Instance = VK_NULL_HANDLE;
	inline static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
	inline static VkDevice                 g_Device = VK_NULL_HANDLE;
	inline static uint32_t                 g_QueueFamily = (uint32_t)-1;
	inline static VkQueue                  g_Queue = VK_NULL_HANDLE;
	inline static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
	inline static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
	inline static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

	inline static int                      g_MinImageCount = 2;
	inline static bool                     g_SwapChainRebuild = false;

  MyVulkanDevice(const MyVulkanDevice &) = delete;
  MyVulkanDevice& operator=(const MyVulkanDevice &) = delete;
  MyVulkanDevice(MyVulkanDevice &&) = delete;
  MyVulkanDevice &operator=(MyVulkanDevice &&) = delete;

  VkCommandPool getCommandPool() { return commandPool; }
  VkDevice device() { return device_; }
  VkSurfaceKHR surface() { return surface_; }
  VkQueue graphicsQueue() { return graphicsQueue_; }
  VkQueue presentQueue() { return presentQueue_; }

  SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
  VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  // Buffer Helper Functions
  void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);
  void TransitionImageLayout(VkImage image, VkFormat format, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout);
  void GenerateMipmaps(VkImage image,VkFormat format, int32_t texWidht, int32_t texHeight, uint32_t mipLevels);
  VkSampleCountFlagBits GetMaxUsableSampleCount();

  VkPhysicalDeviceProperties properties;

  DeletionQueue deletionQueue;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VulkanInit& myWindow;
  VkCommandPool commandPool;
  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;
  VkSampleCountFlagBits maxSamples = VK_SAMPLE_COUNT_1_BIT;

  VkDevice device_;
  VkSurfaceKHR surface_;
  VkQueue graphicsQueue_;
  VkQueue presentQueue_;
  VkQueue computeQueue_;

	 private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);



  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
/*                                                      VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                                      VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                                                      VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                                                      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                                      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                                      VK_KHR_SPIRV_1_4_EXTENSION_NAME,
                                                       VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME */};
  														



  /*RAYTRACING FEATURRESS*/



  VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
  static const bool enableRayTracingFeatures = false;


  void createLogicalDeviceRT();
  void initRayTracing();

};

}  