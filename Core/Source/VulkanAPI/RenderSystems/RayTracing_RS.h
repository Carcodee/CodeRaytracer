#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/DescriptorSetHandler/MyDescriptorSets.h"
#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include "VulkanAPI/VulkanObjects/Buffers/Buffer.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include "VulkanAPI/Utility/Utility.h"
namespace VULKAN {
	// Holds data for a ray tracing scratch buffer that is used as a temporary storage
	struct RayTracingScratchBuffer
	{
		uint64_t deviceAddress = 0;
		VkBuffer handle = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
	};

	// Ray tracing acceleration structure
	struct AccelerationStructure {
		VkAccelerationStructureKHR handle;
		uint64_t deviceAddress = 0;
		VkDeviceMemory memory;
		VkBuffer buffer;
	};


	class RayTracing_RS
	{
	public:

		RayTracing_RS(MyVulkanDevice& device, VulkanRenderer& renderer);

		PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
		PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
		PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

		bool readyToDraw = false;

		VKTexture* storageImage;
		void Create_RT_RenderSystem();
		void DrawRT(VkCommandBuffer& currentBuffer);
		void TransitionStorageImage();
	private:
		struct UniformData {
			glm::mat4 viewInverse;
			glm::mat4 projInverse;
		} uniformData;


		//helpers
		void LoadFunctionsPointers();
		RayTracingScratchBuffer CreateScratchBuffer(VkDeviceSize size);
		void DeleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer);
		void CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
		uint64_t getBufferDeviceAddress(VkBuffer buffer);
		void CreateStorageImage();
		void CreateBottomLevelAccelerationStructure();
		void CreateTopLevelAccelerationStructure();
		void CreateShaderBindingTable();
		void CreateDescriptorSets();
		void CreateRTPipeline();
		void CreateUniformBuffer();
		void UpdateUniformbuffers();

		VulkanRenderer& myRenderer;
		MyVulkanDevice& myDevice;

		AccelerationStructure bottomLevelAS{};
		AccelerationStructure topLevelAS{};
		
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};


		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		uint32_t indexCount;

		Buffer vertexBuffer;
		Buffer indexBuffer;
		Buffer transformBuffer;
		Buffer raygenShaderBindingTable;
		Buffer missShaderBindingTable;
		Buffer hitShaderBindingTable;
		Buffer ubo;


		VkShaderModule rHitShaderModule;
		VkShaderModule rMissShaderModule;
		VkShaderModule rGenShaderModule;

	};

}


