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


	struct RxTexture
	{
		Material mat;
		VkDescriptorSet descriptorSet;
	};

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
		struct GeometryData
		{
			uint64_t vertexBufferDeviceAddress;
			uint64_t indexBufferDeviceAddress;
			int textureIndexBaseColor;
			int textureIndexOcclusion;
		};
		struct BottomLevelObj
		{
			std::vector<GeometryData> geometryData;
			ModelData combinedMesh;
			VkTransformMatrixKHR instanceMatrix;
			VkTransformMatrixKHR matrix;
			std::vector<std::reference_wrapper<VkAccelerationStructureKHR>> totalTopLevelHandles;
			AccelerationStructure BottomLevelAs;
			std::vector<RxTexture> materialsData;

			glm::vec3 pos;
			glm::vec3 rot;
			glm::vec3 scale;
			void UpdateMatrix(){
				instanceMatrix = {
			scale.x, 0.0f, 0.0f, pos.x,
			0.0f, scale.y, 0.0f, pos.y,
			0.0f, 0.0f, scale.z,pos.z};
			}

			int bottomLevelId=0;
		}bottomLevelObj;

		struct TopLevelObj
		{
			std::vector<Vertex>vertices;
			std::vector<uint32_t>indices;

			VkTransformMatrixKHR matrix;
			AccelerationStructure TopLevelAsData;
			std::vector < BottomLevelObj*> bottomLevelObjRef;
			
			glm::vec3 pos;
			glm::vec3 rot;
			glm::vec3 scale;
			int topLevelInstanceCount = 1;
			int TLASID = 0;
			void UpdateMatrix(){
				matrix = {
			scale.x, 0.0f, 0.0f, pos.x,
			0.0f, scale.y, 0.0f, pos.y,
			0.0f, 0.0f, -scale.z,pos.z};
			}
		}topLevelObjBase;


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
		Camera cam{glm::vec3(1.0f, 1.0f, 1.0f)};
		Light light{glm::vec3(0), glm::vec3(1.0f), 1.0f };

		void Create_RT_RenderSystem();
		void DrawRT(VkCommandBuffer& currentBuffer);
		void TransitionStorageImage();

	private:
		struct UniformData {
			glm::mat4 viewInverse;
			glm::mat4 projInverse;
		} uniformData;



		std::vector<ModelData>modelDatas;
		//helpers
		void SetupBottomLevelObj();
		void LoadFunctionsPointers();
		void UpdateUniformbuffers();
		RayTracingScratchBuffer CreateScratchBuffer(VkDeviceSize size);
		void DeleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer);
		void CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
		uint64_t getBufferDeviceAddress(VkBuffer buffer);
		void CreateStorageImage();
		void CreateBottomLevelAccelerationStructureModel(BottomLevelObj& obj);
		void CreateTopLevelAccelerationStructure(TopLevelObj& topLevelObj);
		void CreateShaderBindingTable();
		void CreateDescriptorSets();
		void CreateRTPipeline();
		void CreateUniformBuffer();
		uint32_t GetShaderBindAdress(uint32_t hitGroupStart, uint32_t start, uint32_t offset, uint32_t stbRecordOffset, uint32_t geometryIndex, uint32_t stbRecordStride);

		VulkanRenderer& myRenderer;
		MyVulkanDevice& myDevice;
		ModelLoaderHandler modelLoader{ myDevice };

		//AccelerationStructure bottomLevelAS{};
		//AccelerationStructure topLevelAS{};
		
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
		Buffer combinedMeshBuffer;
		Buffer indexBuffer;
		Buffer transformBuffer;
		Buffer raygenShaderBindingTable;
		Buffer missShaderBindingTable;
		Buffer hitShaderBindingTable;
		Buffer ubo;
		Buffer lightBuffer;


		VkShaderModule rHitShaderModule;
		VkShaderModule rMissShaderModule;
		VkShaderModule rGenShaderModule;

	};

}


