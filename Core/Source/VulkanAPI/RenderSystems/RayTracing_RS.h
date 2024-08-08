#pragma once
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/DescriptorSetHandler/MyDescriptorSets.h"
#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include "VulkanAPI/VulkanObjects/Buffers/Buffer.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"
#include "VulkanAPI/Utility/Utility.h"
#include "VulkanAPI/Camera/Camera.h"
#include "VulkanAPI/VulkanObjects/Buffers/VKBufferHandler.h"
#include <random>


namespace VULKAN {
	// Holds data for a ray tracing scratch buffer that is used as a temporary storage

	struct Light 
	{
		alignas(16)glm::vec3 pos;
		alignas(16)glm::vec3 color;
		alignas(4)float intensity;
		alignas(4)float padding[3];
	};
	class RayTracing_RS
	{
	public:

        BottomLevelObj bottomLevelObj;
        TopLevelObj topLevelObjBase;

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
		bool updateDescriptorData = false;

		VKTexture* storageImage;
        VKTexture* emissiveStoreImage;
		Camera cam{glm::vec3(1.0f, 1.0f, 1.0f)};
		Light light{glm::vec3(0), glm::vec3(1.0f), 1.0f };
        PushConstantBlock_RS pc;

		void Create_RT_RenderSystem();
		void DrawRT(VkCommandBuffer& currentBuffer);
		void AddModelToPipeline(std::shared_ptr<ModelData> modelData);
        void RecreateBLASesAndTLASes();
		void UpdateRaytracingData();
        void UpdateMeshInfo();
        void UpdateAABBsInfo();
        void UpdateMaterialInfo();
        void ResetAccumulatedFrames();

	private:
		struct UniformData {
			glm::mat4 viewInverse;
			glm::mat4 projInverse;
		} uniformData;
        bool runningModels = false;

		std::vector<std::shared_ptr<ModelData>>modelsOnScene;
        std::vector<uint32_t> instancesGeometryOffsets;
		//helpers
        void CleanBuffers();
		void SetupBottomLevelObj(std::shared_ptr<ModelData> modelData);
		void LoadFunctionsPointers();
		void UpdateUniformbuffers();
		RayTracingScratchBuffer CreateScratchBuffer(VkDeviceSize size);
		void DeleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer);
		void CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
		uint64_t getBufferDeviceAddress(VkBuffer buffer);
		void CreateStorageImages();
		void CreateBottomLevelAccelerationStructureModel(BottomLevelObj& obj);
        void CreateBottomLevelAccelerationStructureSpheres(Sphere& sphere);
		void CreateTopLevelAccelerationStructure(TopLevelObj& topLevelObj);
		void CreateShaderBindingTable();
		void CreateDescriptorSets();
		void CreateRTPipeline();
		void CreateUniformBuffer();
		void CreateMaterialsBuffer();
		void CreateAllModelsBuffer();
        void CreateAABBsBuffer();
        void DestroyAccelerationStructure(AccelerationStructure& accelerationStructure);
		uint32_t GetShaderBindAdress(uint32_t hitGroupStart, uint32_t start, uint32_t offset, uint32_t stbRecordOffset, uint32_t geometryIndex, uint32_t stbRecordStride);

		VulkanRenderer& myRenderer;
		MyVulkanDevice& myDevice;

		
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
		std::vector<VkWriteDescriptorSet> writeDescriptorSets{};


		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		uint32_t indexCount;

        //dynamic buffers
		Buffer vertexBuffer;
		Buffer indexBuffer;
		Buffer transformBuffer;
        Buffer BLAsInstanceOffsetBuffer;
        Buffer allMaterialsBuffer;
        Buffer allModelDataBuffer;
        Buffer allSpheresBuffer;
        Buffer allAABBBuffer;
        
        //raytracing
		Buffer raygenShaderBindingTable;
		Buffer missShaderBindingTable;
		Buffer hitShaderBindingTable;
        
        //ubo
		Buffer ubo;
		Buffer lightBuffer;



		VkShaderModule rHitShaderModule;
		VkShaderModule rMissShaderModule;
		VkShaderModule rGenShaderModule;
        
        VKTexture* baseTexture;
        VKTexture* environmentTexture;
        std::vector<AABB> aabbs;
        
	};

}


