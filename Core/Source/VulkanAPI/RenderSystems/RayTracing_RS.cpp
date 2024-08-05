#include "RayTracing_RS.h"

#include "FileSystem/FileHandler.h"
#include "VulkanAPI/Model/ModelHandler.h"
#include "VulkanAPI/Utility/InputSystem/InputHandler.h"


namespace VULKAN {

	RayTracing_RS::RayTracing_RS(MyVulkanDevice& device, VulkanRenderer& renderer):myDevice(device), myRenderer(renderer)
	{
		cam.SetPerspective(100.0f, (float)800 / (float)600, 0.1f, 512.0f);
		cam.position=(glm::vec3(0.0f, 4.0f, 0.0f));
		cam.currentMode = CameraMode::E_Free;
        //cam.SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));
	}
	
	void RayTracing_RS::LoadFunctionsPointers()
	{
		vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkGetBufferDeviceAddressKHR"));
		vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkCmdBuildAccelerationStructuresKHR"));
		vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkBuildAccelerationStructuresKHR"));
		vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkCreateAccelerationStructureKHR"));
		vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkDestroyAccelerationStructureKHR"));
		vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkGetAccelerationStructureBuildSizesKHR"));
		vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkGetAccelerationStructureDeviceAddressKHR"));
		vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkCmdTraceRaysKHR"));
		vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkGetRayTracingShaderGroupHandlesKHR"));
		vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(myDevice.device(), "vkCreateRayTracingPipelinesKHR"));

	}

	RayTracingScratchBuffer  RayTracing_RS::CreateScratchBuffer(VkDeviceSize size)
	{
		RayTracingScratchBuffer scratchBuffer{};

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		if (vkCreateBuffer(myDevice.device(), &bufferCreateInfo, nullptr, &scratchBuffer.handle) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create scratch buffer");
		}
		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(myDevice.device(), scratchBuffer.handle, &memoryRequirements);

		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = myDevice.findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (vkAllocateMemory(myDevice.device(), &memoryAllocateInfo, nullptr, &scratchBuffer.memory) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create scratch buffer");

		}
		if (vkBindBufferMemory(myDevice.device(), scratchBuffer.handle, scratchBuffer.memory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to bind scratch buffer");
		}

		VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
		bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAddressInfo.buffer = scratchBuffer.handle;
		scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(myDevice.device(), &bufferDeviceAddressInfo);

		return scratchBuffer;
	}
	void RayTracing_RS::DeleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer)
	{
		if (scratchBuffer.memory != VK_NULL_HANDLE) {
			vkFreeMemory(myDevice.device(), scratchBuffer.memory, nullptr);
		}
		if (scratchBuffer.handle != VK_NULL_HANDLE) {
			vkDestroyBuffer(myDevice.device(), scratchBuffer.handle, nullptr);
		}
	}

	void RayTracing_RS::CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		if (vkCreateBuffer(myDevice.device(), &bufferCreateInfo, nullptr, &accelerationStructure.buffer)!=VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create Acceleration Structure buffer");
		};
		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(myDevice.device(), accelerationStructure.buffer, &memoryRequirements);
		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = myDevice.findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(myDevice.device(), &memoryAllocateInfo, nullptr, &accelerationStructure.memory) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to Allocate Acceleration Structure memory");
		}
		if (vkBindBufferMemory(myDevice.device(), accelerationStructure.buffer, accelerationStructure.memory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to bind Acceleration Structure memory");
		}
	}
	uint64_t RayTracing_RS::getBufferDeviceAddress(VkBuffer buffer)
	{
		VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
		bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAI.buffer = buffer;
		return vkGetBufferDeviceAddressKHR(myDevice.device(), &bufferDeviceAI);
	}
	void RayTracing_RS::CreateStorageImages()
	{
        unsigned int width = myRenderer.GetSwapchain().width();
        unsigned int  height = myRenderer.GetSwapchain().height();
		storageImage = new VKTexture(myRenderer.GetSwapchain(), width, height,
                                     VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_FORMAT_R8G8B8A8_UNORM, 1);
        emissiveStoreImage = new VKTexture(myRenderer.GetSwapchain(), width, height,
                                           VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_FORMAT_R8G8B8A8_UNORM, 4);
	}

    void RayTracing_RS::CreateBottomLevelAccelerationStructureSpheres(Sphere &sphere) {
        VkDeviceOrHostAddressConstKHR sphereBuffersDeviceAddress{};
        VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

        sphereBuffersDeviceAddress.deviceAddress = getBufferDeviceAddress(allSpheresBuffer.buffer) + sphere.id * sizeof(sphere);
        transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer.buffer) /*+ obj.combinedMesh.transformBLASOffset*/;

        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR; // Using AABBs to represent spheres
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
        accelerationStructureGeometry.geometry.aabbs.data.deviceAddress = sphereBuffersDeviceAddress.deviceAddress;
        accelerationStructureGeometry.geometry.aabbs.stride = sizeof(Sphere);


        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = 1;
        accelerationStructureBuildRangeInfo.primitiveOffset =0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;
        
        VkAccelerationStructureBuildGeometryInfoKHR accelerationsStructureBuildGeometryInfo{};
        accelerationsStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationsStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationsStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationsStructureBuildGeometryInfo.geometryCount = 1;
        accelerationsStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

        uint32_t primitiveCount = 1;
        
        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(
                myDevice.device(),
                VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &accelerationsStructureBuildGeometryInfo,
                &primitiveCount,
                &accelerationStructureBuildSizesInfo);

        CreateAccelerationStructureBuffer(sphere.accelerationStructure, accelerationStructureBuildSizesInfo);

        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
        accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreateInfo.buffer = sphere.accelerationStructure.buffer;
        accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        vkCreateAccelerationStructureKHR(myDevice.device(), &accelerationStructureCreateInfo, nullptr, &sphere.accelerationStructure.handle);

        // Create a small scratch buffer used during build of the bottom level acceleration structure
        RayTracingScratchBuffer scratchBuffer = CreateScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

        accelerationsStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationsStructureBuildGeometryInfo.dstAccelerationStructure = sphere.accelerationStructure.handle;
        accelerationsStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

        const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos[] = { &accelerationStructureBuildRangeInfo };
        VkCommandBuffer commandBuffer = myDevice.beginSingleTimeCommands();

        vkCmdBuildAccelerationStructuresKHR(
                commandBuffer,
                1,
                &accelerationsStructureBuildGeometryInfo,
                pBuildRangeInfos);

        myDevice.endSingleTimeCommands(commandBuffer);
        VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
        accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        accelerationDeviceAddressInfo.accelerationStructure = sphere.accelerationStructure.handle;
        sphere.accelerationStructure.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(myDevice.device(), &accelerationDeviceAddressInfo);
        DeleteScratchBuffer(scratchBuffer);

    }

	void RayTracing_RS::CreateBottomLevelAccelerationStructureModel(BottomLevelObj& obj)
	{
		uint32_t maxPrimCount{ 0 };
		std::vector<uint32_t> maxPrimitiveCounts{};
		std::vector<VkAccelerationStructureGeometryKHR> geometries;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRangeInfos{};
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> pBuildRangeInfos{};
		VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};
		for (int i = 0;i< obj.combinedMesh.meshCount; i++)
		{

			vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer.buffer) + obj.combinedMesh.vertexBLASOffset * sizeof(Vertex);
			indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(indexBuffer.buffer) + ((obj.combinedMesh.indexBLASOffset + obj.combinedMesh.firstMeshIndex[i]) * sizeof(uint32_t));
			transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer.buffer) /*+ obj.combinedMesh.transformBLASOffset*/;

			VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
			accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
			accelerationStructureGeometry.geometry.triangles.maxVertex = static_cast<uint32_t>(obj.combinedMesh.vertices.size());
			accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
			accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
			accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
			accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
			accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;
			geometries.push_back(accelerationStructureGeometry);


			maxPrimCount += obj.combinedMesh.meshIndexCount[i] / 3;
			maxPrimitiveCounts.push_back(obj.combinedMesh.meshIndexCount[i]/3);

			VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
			accelerationStructureBuildRangeInfo.primitiveCount = obj.combinedMesh.meshIndexCount[i]/3;
			accelerationStructureBuildRangeInfo.primitiveOffset =0;
			accelerationStructureBuildRangeInfo.firstVertex = 0;
			accelerationStructureBuildRangeInfo.transformOffset = 0;
			buildRangeInfos.push_back(accelerationStructureBuildRangeInfo);

		}
		for (auto& buildRangeInfo : buildRangeInfos)
		{
			pBuildRangeInfos.push_back(&buildRangeInfo);
		}

		VkAccelerationStructureBuildGeometryInfoKHR accelerationsStructureBuildGeometryInfo{};
		accelerationsStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationsStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationsStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationsStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(geometries.size());
		accelerationsStructureBuildGeometryInfo.pGeometries = geometries.data();

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			myDevice.device(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationsStructureBuildGeometryInfo,
			maxPrimitiveCounts.data(),
			&accelerationStructureBuildSizesInfo);

		CreateAccelerationStructureBuffer(obj.BottomLevelAs, accelerationStructureBuildSizesInfo);

		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = obj.BottomLevelAs.buffer;
		accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		vkCreateAccelerationStructureKHR(myDevice.device(), &accelerationStructureCreateInfo, nullptr, &obj.BottomLevelAs.handle);

		// Create a small scratch buffer used during build of the bottom level acceleration structure
		RayTracingScratchBuffer scratchBuffer = CreateScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

		accelerationsStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationsStructureBuildGeometryInfo.dstAccelerationStructure = obj.BottomLevelAs.handle;
		accelerationsStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		const VkAccelerationStructureBuildRangeInfoKHR* accelerationBuildStructureRangeInfos =  buildRangeInfos.data();
		VkCommandBuffer commandBuffer = myDevice.beginSingleTimeCommands();

		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationsStructureBuildGeometryInfo,
			pBuildRangeInfos.data());

		myDevice.endSingleTimeCommands(commandBuffer);
		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = obj.BottomLevelAs.handle;
		obj.BottomLevelAs.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(myDevice.device(), &accelerationDeviceAddressInfo);

		DeleteScratchBuffer(scratchBuffer);

	}

	void RayTracing_RS::CreateTopLevelAccelerationStructure(TopLevelObj& topLevelObj)
	{
		std::vector<VkAccelerationStructureInstanceKHR> instances;


		for (int i= 0;i < ModelHandler::GetInstance()->GetBLASesFromTLAS(topLevelObj).size(); i++)
		{
			BottomLevelObj& objRef = ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObj, i);
			VkAccelerationStructureInstanceKHR instance{};
		    instance.transform = objRef.instanceMatrix;
		    instance.instanceCustomIndex = 0;
			instance.mask = 0xFF;	
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = objRef.BottomLevelAs.deviceAddress;
			instances.push_back(instance);
		}
        
        for (int i = 0; i <ModelHandler::GetInstance()->allSpheresOnApp.size() ; ++i) {
            Sphere& sphereRef = ModelHandler::GetInstance()->allSpheresOnApp[i];
            VkAccelerationStructureInstanceKHR instance{};
            instance.transform = sphereRef.transformMatrix;
            instance.instanceCustomIndex = 0;
            instance.mask = 0xFF;
            instance.instanceShaderBindingTableRecordOffset = 1;
            instance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
            instance.accelerationStructureReference = sphereRef.accelerationStructure.deviceAddress;
            instances.push_back(instance);
        }
		Buffer instancesBuffer;

		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&instancesBuffer,
			sizeof(VkAccelerationStructureInstanceKHR) *instances.size(),
			instances.data());

		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(instancesBuffer.buffer);

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
		accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
		accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		uint32_t primitive_count = static_cast<uint32_t>(instances.size());

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			myDevice.device(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&primitive_count,
			&accelerationStructureBuildSizesInfo);

		CreateAccelerationStructureBuffer(topLevelObj.TopLevelAsData, accelerationStructureBuildSizesInfo);

		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = topLevelObj.TopLevelAsData.buffer;
		accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		vkCreateAccelerationStructureKHR(myDevice.device(), &accelerationStructureCreateInfo, nullptr, &topLevelObj.TopLevelAsData.handle);

		// Create a small scratch buffer used during build of the top level acceleration structure
		RayTracingScratchBuffer scratchBuffer = CreateScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelObj.TopLevelAsData.handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount =primitive_count;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		VkCommandBuffer commandBuffer = myDevice.beginSingleTimeCommands();

		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());

		myDevice.endSingleTimeCommands(commandBuffer);
		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = topLevelObj.TopLevelAsData.handle;
		topLevelObj.TopLevelAsData.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(myDevice.device(), &accelerationDeviceAddressInfo);

		DeleteScratchBuffer(scratchBuffer);
		instancesBuffer.device = myDevice.device();
		instancesBuffer.destroy();
	}

	void RayTracing_RS::CreateShaderBindingTable()
	{
		const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
		const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
		const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		vkGetRayTracingShaderGroupHandlesKHR(myDevice.device(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data());

		const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		myDevice.createBuffer(bufferUsageFlags, memoryUsageFlags, &raygenShaderBindingTable, handleSize);
		myDevice.createBuffer(bufferUsageFlags, memoryUsageFlags, &missShaderBindingTable, handleSize * 2);
		myDevice.createBuffer(bufferUsageFlags, memoryUsageFlags, &hitShaderBindingTable, handleSize);
        
		// Copy handles
		raygenShaderBindingTable.device = myDevice.device();
		missShaderBindingTable.device = myDevice.device();
		hitShaderBindingTable.device = myDevice.device();

		raygenShaderBindingTable.map();
		missShaderBindingTable.map();
		hitShaderBindingTable.map();
		memcpy(raygenShaderBindingTable.mapped, shaderHandleStorage.data(), handleSize);
		memcpy(missShaderBindingTable.mapped, shaderHandleStorage.data() + (handleSizeAligned), handleSize * 2);
		memcpy(hitShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned * 3, handleSize);
	}

    void RayTracing_RS::RecreateBLASesAndTLASes() {
        for (int i = 0; i < ModelHandler::GetInstance()->GetBLASesFromTLAS(topLevelObjBase).size(); i++)
        {
            CreateBottomLevelAccelerationStructureModel(ModelHandler::GetInstance()->GetBLASesFromTLAS(topLevelObjBase)[i]);
        }
        for (int i = 0; i < ModelHandler::GetInstance()->allSpheresOnApp.size(); ++i) {
            CreateBottomLevelAccelerationStructureSpheres(ModelHandler::GetInstance()->allSpheresOnApp[i]);
        }
        CreateTopLevelAccelerationStructure(topLevelObjBase);

    }
	void RayTracing_RS::CreateDescriptorSets()
	{

		uint32_t imageCount = 1;
		uint32_t materialCount =0;
		uint32_t meshCount =0;
		for (auto model : modelsOnScene)
		{
			materialCount += static_cast<uint32_t>(model->materialDataPerMesh.size());
			meshCount += static_cast<uint32_t>(model->meshCount);
		}
		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
		};
        
        std::string texPath=HELPERS::FileHandler::GetInstance()->GetAssetsPath()+"/Images/Solid_white.png";
        std::string environmentPath=HELPERS::FileHandler::GetInstance()->GetEngineResourcesPath()+"/Images/Env.hdr";
		baseTexture = new VKTexture(texPath.c_str(), myRenderer.GetSwapchain());
        environmentTexture = new VKTexture(environmentPath.c_str(), myRenderer.GetSwapchain());


		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = INITIALIZERS::descriptorPoolCreateInfo(poolSizes, 1);
		if (vkCreateDescriptorPool(myDevice.device(), &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create descriptorPools");
		}
		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorCountAllocInfo{};
		uint32_t variableDescCounts[] = {  imageCount };
		variableDescriptorCountAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
		variableDescriptorCountAllocInfo.descriptorSetCount = 1;
		variableDescriptorCountAllocInfo.pDescriptorCounts = variableDescCounts;


		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =INITIALIZERS::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
		descriptorSetAllocateInfo.pNext = &variableDescriptorCountAllocInfo;

		if (vkAllocateDescriptorSets(myDevice.device(), &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create descriptorAllocateInfo");
		}
        
        VkAccelerationStructureKHR nullAccelerationStructure = VK_NULL_HANDLE;
        
		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &nullAccelerationStructure;

		VkWriteDescriptorSet accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureWrite.dstSet = descriptorSet;
		accelerationStructureWrite.dstBinding = 0;
		accelerationStructureWrite.descriptorCount = 1;
		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        

		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = storageImage->textureImageView;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkDescriptorImageInfo roomText{};
		roomText.imageView =baseTexture->textureImageView;
		roomText.sampler = baseTexture->textureSampler;
		roomText.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		ubo.descriptor.buffer = ubo.buffer;
		ubo.descriptor.offset = 0;
		ubo.descriptor.range = sizeof(UniformData);
		 
		lightBuffer.descriptor.buffer = lightBuffer.buffer;
		lightBuffer.descriptor.offset = 0;
		lightBuffer.descriptor.range = sizeof(Light);

		vertexBuffer.descriptor.buffer = vertexBuffer.buffer;
		vertexBuffer.descriptor.offset = 0;
		vertexBuffer.descriptor.range =  VK_WHOLE_SIZE;

		indexBuffer.descriptor.buffer = indexBuffer.buffer;
		indexBuffer.descriptor.offset = 0;
		indexBuffer.descriptor.range =  VK_WHOLE_SIZE;

		allMaterialsBuffer.descriptor.buffer = allMaterialsBuffer.buffer;
		allMaterialsBuffer.descriptor.offset = 0;
		allMaterialsBuffer.descriptor.range = VK_WHOLE_SIZE;

		allModelDataBuffer.descriptor.buffer = allModelDataBuffer.buffer;
		allModelDataBuffer.descriptor.offset = 0;
		allModelDataBuffer.descriptor.range = VK_WHOLE_SIZE;

        BLAsInstanceOffsetBuffer.descriptor.buffer = BLAsInstanceOffsetBuffer.buffer;
        BLAsInstanceOffsetBuffer.descriptor.offset = 0;
        BLAsInstanceOffsetBuffer.descriptor.range = VK_WHOLE_SIZE;

        VkDescriptorImageInfo emissiveStorageDescriptor{};
        emissiveStorageDescriptor.imageView = emissiveStoreImage->textureImageView;
        emissiveStorageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        
        VkDescriptorImageInfo environmentTextureImage{};
        environmentTextureImage.imageView =environmentTexture->textureImageView;
		environmentTextureImage.sampler = environmentTexture->textureSampler;
		environmentTextureImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        VkWriteDescriptorSet resultImageWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor);
		VkWriteDescriptorSet uniformBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &ubo.descriptor);
		VkWriteDescriptorSet textWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &roomText);
		VkWriteDescriptorSet vertexBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &vertexBuffer.descriptor);
		VkWriteDescriptorSet indexBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5, &indexBuffer.descriptor);
		VkWriteDescriptorSet lightBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6, &lightBuffer.descriptor);
		VkWriteDescriptorSet allMaterialsBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 7, &allMaterialsBuffer.descriptor);
		VkWriteDescriptorSet allModelsDataBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8, &allModelDataBuffer.descriptor);
        VkWriteDescriptorSet BLAsInstanceOffsetBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 9, &BLAsInstanceOffsetBuffer.descriptor);
        VkWriteDescriptorSet emissiveImageWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10, &emissiveStorageDescriptor);
        VkWriteDescriptorSet environmentImageWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 11, &environmentTextureImage);

        std::vector<VkDescriptorImageInfo> texturesDescriptors{};
   
        if (texturesDescriptors.size()<=0)
        {
            VkDescriptorImageInfo descriptor{};
            descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptor.sampler = baseTexture->textureSampler;
            descriptor.imageView = baseTexture->textureImageView;
            texturesDescriptors.push_back(descriptor);
        }
        VkWriteDescriptorSet writeDescriptorImgArray{};
        writeDescriptorImgArray.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorImgArray.dstBinding = 12;
        writeDescriptorImgArray.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorImgArray.descriptorCount = imageCount;
        writeDescriptorImgArray.dstSet = descriptorSet;
        writeDescriptorImgArray.pImageInfo = texturesDescriptors.data();
		
		writeDescriptorSets = {
			accelerationStructureWrite,
			resultImageWrite,
			uniformBufferWrite,
			textWrite,
			vertexBufferWrite,
			indexBufferWrite,
			lightBufferWrite,
			allMaterialsBufferWrite,
			allModelsDataBufferWrite,
            BLAsInstanceOffsetBufferWrite,
            emissiveImageWrite,
            environmentImageWrite,
            writeDescriptorImgArray
		};
	   

//        assert(false);
		vkUpdateDescriptorSets(myDevice.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
	}

	void RayTracing_RS::UpdateRaytracingData()
	{
        ModelHandler::GetInstance()->CreateMaterialTextures(myRenderer.GetSwapchain());
        CreateMaterialsBuffer();
        CreateAllModelsBuffer();
        RecreateBLASesAndTLASes();
        
		uint32_t imageCount = 0;
        uint32_t materialCount =0;
		if (ModelHandler::GetInstance()->allMaterialsOnApp.size()>0)
		{
			for (auto& mat : ModelHandler::GetInstance()->allMaterialsOnApp)
			{
                uint32_t size =static_cast<uint32_t>(mat.second->materialUniform.texturesSizes);
				imageCount += size;
                materialCount ++/*= static_cast<uint32_t>(model.materialDataPerMesh.size())*/;
			}
		}
		if (imageCount==0)
		{
            std::cout<<"There is no images on the materials loaded!"<<"\n";
			imageCount = 1;
		}
		uint32_t meshCount =0;

		for (auto model : modelsOnScene)
		{
			meshCount += static_cast<uint32_t>(model->meshCount);
		}

		if (materialCount == 0)
		{
			materialCount = 1;
		}
        std::vector<VkDescriptorPoolSize> poolSizes = {
                { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
        };

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = INITIALIZERS::descriptorPoolCreateInfo(poolSizes, 1);
        if (vkCreateDescriptorPool(myDevice.device(), &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create descriptorPools");
        } 
        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorCountAllocInfo{};
		uint32_t variableDescCounts[] = {  imageCount };
		variableDescriptorCountAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
		variableDescriptorCountAllocInfo.descriptorSetCount = 1;
		variableDescriptorCountAllocInfo.pDescriptorCounts = variableDescCounts;


		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =INITIALIZERS::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
		descriptorSetAllocateInfo.pNext = &variableDescriptorCountAllocInfo;

        
		if (vkAllocateDescriptorSets(myDevice.device(), &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create descriptorAllocateInfo");
		}
        
        
		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;

		descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelObjBase.TopLevelAsData.handle;

		VkWriteDescriptorSet accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureWrite.dstSet = descriptorSet;
		accelerationStructureWrite.dstBinding = 0;
		accelerationStructureWrite.descriptorCount = 1;
		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = storageImage->textureImageView;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkDescriptorImageInfo roomText{};
		roomText.imageView =baseTexture->textureImageView;
		roomText.sampler = baseTexture->textureSampler;
		roomText.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		uint32_t vertexSize = 0;
		uint32_t indexSize = 0;
		for (int i = 0; i < ModelHandler::GetInstance()->GetBLASesFromTLAS(topLevelObjBase).size(); ++i)
		{
			vertexSize += static_cast<uint32_t>(ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, i).combinedMesh.vertices.size());
			indexSize += static_cast<uint32_t>(ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, i).combinedMesh.indices.size());
		}

		ubo.descriptor.buffer = ubo.buffer;
		ubo.descriptor.offset = 0;
		ubo.descriptor.range = sizeof(UniformData);
		 
		lightBuffer.descriptor.buffer = lightBuffer.buffer;
		lightBuffer.descriptor.offset = 0;
		lightBuffer.descriptor.range = sizeof(Light);

		vertexBuffer.descriptor.buffer = vertexBuffer.buffer;
		vertexBuffer.descriptor.offset = 0;
		vertexBuffer.descriptor.range = sizeof(Vertex) * vertexSize;

		indexBuffer.descriptor.buffer = indexBuffer.buffer;
		indexBuffer.descriptor.offset = 0;
		indexBuffer.descriptor.range = sizeof(uint32_t) * indexSize;

		allMaterialsBuffer.descriptor.buffer = allMaterialsBuffer.buffer;
		allMaterialsBuffer.descriptor.offset = 0;
		allMaterialsBuffer.descriptor.range = sizeof(MaterialUniformData) * materialCount;

		allModelDataBuffer.descriptor.buffer = allModelDataBuffer.buffer;
		allModelDataBuffer.descriptor.offset = 0;
		allModelDataBuffer.descriptor.range = sizeof(ModelDataUniformBuffer) * meshCount;
        
        BLAsInstanceOffsetBuffer.descriptor.buffer = BLAsInstanceOffsetBuffer.buffer;
        BLAsInstanceOffsetBuffer.descriptor.offset = 0;
        BLAsInstanceOffsetBuffer.descriptor.range = sizeof(uint32_t) * instancesGeometryOffsets.size();

        VkDescriptorImageInfo emissiveStorageDescriptor{};
        emissiveStorageDescriptor.imageView = emissiveStoreImage->textureImageView;
        emissiveStorageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
 
        VkDescriptorImageInfo environmentTextureImage{};
        environmentTextureImage.imageView =environmentTexture->textureImageView;
		environmentTextureImage.sampler = environmentTexture->textureSampler;
		environmentTextureImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet resultImageWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor);
		VkWriteDescriptorSet uniformBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &ubo.descriptor);
		VkWriteDescriptorSet textWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &roomText);
		VkWriteDescriptorSet vertexBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &vertexBuffer.descriptor);
		VkWriteDescriptorSet indexBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5, &indexBuffer.descriptor);
		VkWriteDescriptorSet lightBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6, &lightBuffer.descriptor);
		VkWriteDescriptorSet allMaterialsBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 7, &allMaterialsBuffer.descriptor);
		VkWriteDescriptorSet allModelsDataBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8, &allModelDataBuffer.descriptor);
        VkWriteDescriptorSet BLAsInstanceOffsetBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 9, &BLAsInstanceOffsetBuffer.descriptor);
        VkWriteDescriptorSet emissiveImageWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10, &emissiveStorageDescriptor);
        VkWriteDescriptorSet environmentImageWrite = INITIALIZERS::writeDescriptorSet(descriptorSet,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 11, &environmentTextureImage);
        
		std::vector<VkDescriptorImageInfo> texturesDescriptors{};
		if (ModelHandler::GetInstance()->allMaterialsOnApp.size()>0)
		{
			for (auto& mat: ModelHandler::GetInstance()->allMaterialsOnApp)
			{
                std::map<TEXTURE_TYPE,VKTexture*>& currentTextures =mat.second->materialTextures;
                for (auto& pair : currentTextures) {
                    
                    VkDescriptorImageInfo descriptor{};
                    descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    descriptor.sampler = pair.second->textureSampler;
                    descriptor.imageView = pair.second->textureImageView;
                    texturesDescriptors.push_back(descriptor);

                }
               
			}

		}
        if (texturesDescriptors.size()<=0)
        {
            VkDescriptorImageInfo descriptor{};
            descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptor.sampler = baseTexture->textureSampler;
            descriptor.imageView = baseTexture->textureImageView;
            texturesDescriptors.push_back(descriptor);
        }
        VkWriteDescriptorSet writeDescriptorImgArray{};
        writeDescriptorImgArray.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorImgArray.dstBinding = 12;
        writeDescriptorImgArray.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorImgArray.descriptorCount = imageCount;
        writeDescriptorImgArray.dstSet = descriptorSet;
        writeDescriptorImgArray.pImageInfo = texturesDescriptors.data();

        writeDescriptorSets = {
			accelerationStructureWrite,
			resultImageWrite,
			uniformBufferWrite,
			textWrite,
			vertexBufferWrite,
			indexBufferWrite,
			lightBufferWrite,
			allMaterialsBufferWrite,
			allModelsDataBufferWrite,
            BLAsInstanceOffsetBufferWrite,
            emissiveImageWrite,
            environmentImageWrite,
            writeDescriptorImgArray
		};

		vkUpdateDescriptorSets(myDevice.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
        ResetAccumulatedFrames();

	}

	void RayTracing_RS::CreateRTPipeline()
	{
		uint32_t imageCount = 0;
		if (ModelHandler::GetInstance()->allMaterialsOnApp.size() > 0)
		{
			imageCount = { static_cast<uint32_t>(ModelHandler::GetInstance()->allMaterialsOnApp[0].get()->materialUniform.texturesSizes) };
		}


		VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
		accelerationStructureLayoutBinding.binding = 0;
		accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		accelerationStructureLayoutBinding.descriptorCount = 1;
		accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
		resultImageLayoutBinding.binding = 1;
		resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		resultImageLayoutBinding.descriptorCount = 1;
		resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;


		VkDescriptorSetLayoutBinding uniformBufferBinding{};
		uniformBufferBinding.binding = 2;
		uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferBinding.descriptorCount = 1;
		uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding TextureBinding{};
		TextureBinding.binding = 3;
		TextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		TextureBinding.descriptorCount =1;
		TextureBinding.stageFlags =  VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding vertexBinding{};
		vertexBinding.binding = 4;
		vertexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vertexBinding.descriptorCount = 1;
		vertexBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding indexBinding{};
		indexBinding.binding = 5;
		indexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		indexBinding.descriptorCount = 1;
		indexBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding lightBinding{};
		lightBinding.binding = 6;
		lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightBinding.descriptorCount = 1;
		lightBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding materialBinding{};
		materialBinding.binding = 7;
		materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		materialBinding.descriptorCount = 1;
		materialBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;;

		VkDescriptorSetLayoutBinding modelDataBinding{};
		modelDataBinding.binding = 8;
		modelDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		modelDataBinding.descriptorCount = 1;
		modelDataBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

        VkDescriptorSetLayoutBinding instancesGeometryOffsetsBinding{};
        instancesGeometryOffsetsBinding.binding = 9;
        instancesGeometryOffsetsBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        instancesGeometryOffsetsBinding.descriptorCount = 1;
        instancesGeometryOffsetsBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        
        VkDescriptorSetLayoutBinding emissiveImageBinding{};
        emissiveImageBinding.binding = 10;
        emissiveImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        emissiveImageBinding.descriptorCount = 1;
        emissiveImageBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        
        VkDescriptorSetLayoutBinding environmentImageBinding{};
        environmentImageBinding.binding = 11;
        environmentImageBinding.descriptorType =  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        environmentImageBinding.descriptorCount = 1;
        environmentImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;



        VkDescriptorSetLayoutBinding texturesBinding{};
		texturesBinding.binding = 12;
		texturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturesBinding.descriptorCount = 10000;
		texturesBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

		



		std::vector<VkDescriptorSetLayoutBinding> bindings({
			accelerationStructureLayoutBinding,
			resultImageLayoutBinding,
			uniformBufferBinding,
			TextureBinding,
			vertexBinding,
			indexBinding,
			lightBinding,
			materialBinding,
			modelDataBinding,
            instancesGeometryOffsetsBinding,
            emissiveImageBinding,
            environmentImageBinding,
			texturesBinding
			});



		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT setLayoutBindingFlags{};
		setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		setLayoutBindingFlags.bindingCount = 13;
		std::vector<VkDescriptorBindingFlagsEXT> descriptorBindingFlags = {
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
            0,
            0,
            0,
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
		};
		setLayoutBindingFlags.pBindingFlags = descriptorBindingFlags.data();

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = INITIALIZERS::descriptorSetLayoutCreateInfo(bindings);
		descriptorSetLayoutCI.pNext = &setLayoutBindingFlags;

		if (vkCreateDescriptorSetLayout(myDevice.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayout)!=VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create descriptor set layouts");
		}
        
        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantBlock_RS);
        
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutCI.pushConstantRangeCount = 1;
        
		if (vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create pipeline layout");
		}

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::string path;
        
        VkSpecializationMapEntry specializationMapEntry = INITIALIZERS::specializationMapEntry(0, 0, sizeof(uint32_t));
        uint32_t maxRecursion = 5;
        VkSpecializationInfo specializationInfo = INITIALIZERS::specializationInfo(1, &specializationMapEntry, sizeof(maxRecursion), &maxRecursion);

        std::string shaderPath= HELPERS::FileHandler::GetInstance()->GetShadersPath();
		// Ray generation group
		{
			path = shaderPath + "/RayTracingShaders/raygen.rgen.spv";
			shaderStages.push_back(PipelineReader::CreateShaderStageModule(rGenShaderModule,myDevice,VK_SHADER_STAGE_RAYGEN_BIT_KHR, path));
            shaderStages.back().pSpecializationInfo= &specializationInfo;
			VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			this->shaderGroups.push_back(shaderGroup);
		}

		// Miss group
		{
			path = shaderPath+ "/RayTracingShaders/miss.rmiss.spv";
			shaderStages.push_back(PipelineReader::CreateShaderStageModule(rMissShaderModule, myDevice, VK_SHADER_STAGE_MISS_BIT_KHR, path));
			VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			this->shaderGroups.push_back(shaderGroup);
            
            path = shaderPath+ "/RayTracingShaders/shadow.rmiss.spv";
            shaderStages.push_back(PipelineReader::CreateShaderStageModule(rMissShaderModule, myDevice, VK_SHADER_STAGE_MISS_BIT_KHR, path));
            shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
            this->shaderGroups.push_back(shaderGroup);
		}

		// Closest hit group
		{
			path = shaderPath+ "/RayTracingShaders/closesthit.rchit.spv";
			shaderStages.push_back(PipelineReader::CreateShaderStageModule(rHitShaderModule, myDevice, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, path));
			VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			this->shaderGroups.push_back(shaderGroup);
		}
        
       
		VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
		rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		rayTracingPipelineCI.pStages = shaderStages.data();
		rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
		rayTracingPipelineCI.pGroups = shaderGroups.data();
		rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
		rayTracingPipelineCI.layout = pipelineLayout;
		if(vkCreateRayTracingPipelinesKHR(myDevice.device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline)!=VK_SUCCESS)
		{
			throw std::runtime_error("Unable to raytracing pipeline ");

		}
	}

	void RayTracing_RS::CreateUniformBuffer()
	{
		myDevice.createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&ubo,
			sizeof(uniformData),
			&uniformData);
		ubo.map();
		myDevice.createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&lightBuffer,
			sizeof(light),
			&light);
		lightBuffer.map();
		UpdateUniformbuffers();
	}

	void RayTracing_RS::CreateMaterialsBuffer()
	{
		std::vector<MaterialUniformData> materialDatas{};

		for (auto& mat:ModelHandler::GetInstance()->allMaterialsOnApp)
		{
            materialDatas.push_back(mat.second->materialUniform);
		}

		myDevice.createBuffer(
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&allMaterialsBuffer,
		materialDatas.size() * sizeof(MaterialUniformData),
		materialDatas.data());
        allMaterialsBuffer.map();
	}

	void RayTracing_RS::CreateAllModelsBuffer()
	{
		std::vector<ModelDataUniformBuffer>modelDataUniformBuffer;
		std::vector <uint32_t> allModelIndices;
		std::vector <Vertex> allModelsVertices;
		std::vector <VkTransformMatrixKHR> allModelsTransformationMatrices;
        std::vector <VkTransformMatrixKHR> allSphereTransformationMatrices;
        instancesGeometryOffsets.clear();

		uint32_t perModelIndexStride=0;
		uint32_t perModelVertexCount = 0;
		uint32_t transformationMatrixCount = 0;
        uint32_t materialOffset = 0;
        uint32_t instanceMeshCountOffset= 0;

		for (int i = 0; i < modelsOnScene.size(); ++i)
		{
            modelsOnScene[i]->dataUniformBuffer.clear();
			for (int j=0 ; j < modelsOnScene[i]->meshCount ; j++)
			{
                ModelDataUniformBuffer currentModelDataUniformBuffer{};
                currentModelDataUniformBuffer.materialIndex = modelsOnScene[i]->materialIds[j];
                currentModelDataUniformBuffer.geometryIndexStart =perModelIndexStride + modelsOnScene[i]->firstMeshIndex[j];
                currentModelDataUniformBuffer.indexOffset = perModelVertexCount;
                modelsOnScene[i]->dataUniformBuffer.push_back(currentModelDataUniformBuffer);
                modelDataUniformBuffer.push_back(currentModelDataUniformBuffer);
				//modelDataUniformBuffer.insert(modelDataUniformBuffer.begin(),myModelDataUniformBuffer);
			}
            instancesGeometryOffsets.push_back(instanceMeshCountOffset);
            instanceMeshCountOffset += modelsOnScene[i]->meshCount;
            materialOffset += modelsOnScene[i]->materialDataPerMesh.size();
			modelsOnScene[i]->indexBLASOffset = perModelIndexStride;
			modelsOnScene[i]->vertexBLASOffset = perModelVertexCount;
			modelsOnScene[i]->transformBLASOffset = i;
            modelsOnScene[i]->bottomLevelObjRef = &ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, i);
			ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, i).combinedMesh.indexBLASOffset = perModelIndexStride;
			ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, i).combinedMesh.vertexBLASOffset = perModelVertexCount;
            
//            ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, i).combinedMesh.transformBLASOffset = i;
			perModelIndexStride += modelsOnScene[i]->indices.size();
			perModelVertexCount += modelsOnScene[i]->vertices.size();
			transformationMatrixCount++;
		}


        for (int i = 0; i < ModelHandler::GetInstance()->GetBLASesFromTLAS(topLevelObjBase).size(); ++i)
        {
            allModelsTransformationMatrices.push_back(ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, i).matrix);
        }
        for (int i = 0; i <ModelHandler::GetInstance()->allSpheresOnApp.size() ; ++i) {
            allSphereTransformationMatrices.push_back(ModelHandler::GetInstance()->allSpheresOnApp[i].transformMatrix);
        }

		allModelIndices.reserve(perModelIndexStride);
		allModelsVertices.reserve(perModelVertexCount);
//        for (int i = 0; i < modelsOnScene.size(); ++i) {
//            for (int j = 0; j < modelsOnScene[i]->indices.size(); ++j) {
//               allModelIndices.push_back(modelsOnScene[i]->indexBLASOffset+modelsOnScene[i]->indices[j]);
//            }
//        }
        for (int i = 0; i < modelsOnScene.size(); ++i)
		{
			allModelIndices.insert(allModelIndices.end(), modelsOnScene[i]->indices.begin(), modelsOnScene[i]->indices.end());
			allModelsVertices.insert(allModelsVertices.end(), modelsOnScene[i]->vertices.begin(), modelsOnScene[i]->vertices.end());
		}

        //shaders

        if (!ModelHandler::GetInstance()->allSpheresOnApp.empty()){
            myDevice.createBuffer(
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &allSpheresBuffer,
                    ModelHandler::GetInstance()->allSpheresOnApp.size() * sizeof(Sphere),
                    ModelHandler::GetInstance()->allSpheresOnApp.data());
            allSpheresBuffer.map();

            myDevice.createBuffer(
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &sphereTransformBuffer,
                    allSphereTransformationMatrices.size() * sizeof(VkTransformMatrixKHR),
                    allModelsTransformationMatrices.data());

        }

            
        myDevice.createBuffer(
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&allModelDataBuffer,
		modelDataUniformBuffer.size() * sizeof(ModelDataUniformBuffer),
		modelDataUniformBuffer.data());
        allModelDataBuffer.map();
        
		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vertexBuffer,
            allModelsVertices.size() * sizeof(Vertex),
		    allModelsVertices.data());

        //this is done because we need to tell the shaders how much offset we need depending the instance becuase geometries restart on every bottom level obj
        myDevice.createBuffer(
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &BLAsInstanceOffsetBuffer,
                instancesGeometryOffsets.size() * sizeof(uint32_t),
                instancesGeometryOffsets.data());
        
        
		// Index buffer
		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&indexBuffer,
            allModelIndices.size() * sizeof(uint32_t),
			allModelIndices.data());
        
		// Transform buffer
		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&transformBuffer,
			allModelsTransformationMatrices.size() * sizeof(VkTransformMatrixKHR),
			allModelsTransformationMatrices.data());

	}

    void RayTracing_RS::UpdateMaterialInfo() {
        
        std::vector<MaterialUniformData> materialDatas{};
        for (auto& mat:ModelHandler::GetInstance()->allMaterialsOnApp)
        {
            materialDatas.push_back(mat.second->materialUniform);
        }
        memcpy(allMaterialsBuffer.mapped, materialDatas.data(), sizeof (MaterialUniformData)* materialDatas.size());
        ResetAccumulatedFrames();

    }
    void RayTracing_RS::UpdateMeshInfo() {
        std::vector<ModelDataUniformBuffer>modelDataUniformBuffer;
        for (int i = 0; i < modelsOnScene.size(); ++i) {
            modelsOnScene[i]->CalculateMaterialsIdsOffsets();
            for (int j = 0; j < modelsOnScene[i]->meshCount; j++) {
                modelDataUniformBuffer.push_back(modelsOnScene[i]->dataUniformBuffer[j]);
            }
        }
        memcpy(allModelDataBuffer.mapped, modelDataUniformBuffer.data(), sizeof (ModelDataUniformBuffer)* modelDataUniformBuffer.size());
        ResetAccumulatedFrames();
    }


    void RayTracing_RS::AddModelToPipeline(std::shared_ptr<ModelData> modelData)
	{
		SetupBottomLevelObj(modelData);
		updateDescriptorData = true;
	}


	uint32_t RayTracing_RS::GetShaderBindAdress(uint32_t hitGroupStart, uint32_t start, uint32_t offset,
	                                            uint32_t stbRecordOffset, uint32_t geometryIndex, uint32_t stbRecordStride)
	{
		return hitGroupStart + start * (offset + stbRecordOffset + (geometryIndex + stbRecordStride));
	}

	void RayTracing_RS::DrawRT(VkCommandBuffer& currentBuffer)
	{

        
        emissiveStoreImage->TransitionTexture( VK_IMAGE_LAYOUT_GENERAL,VK_ACCESS_SHADER_WRITE_BIT,VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, currentBuffer);
        storageImage->TransitionTexture( VK_IMAGE_LAYOUT_GENERAL,VK_ACCESS_SHADER_WRITE_BIT,VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, currentBuffer);
    
		VkImageSubresourceRange imageSubresourceRange = {};
		imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceRange.baseMipLevel = 0;
		imageSubresourceRange.levelCount = 1; 
		imageSubresourceRange.baseArrayLayer = 0;
		imageSubresourceRange.layerCount = 1; 
		const VkClearColorValue clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
        if (InputHandler::editingGraphics){
            ResetAccumulatedFrames();
//            vkCmdClearColorImage(currentBuffer, storageImage->textureImage, VK_IMAGE_LAYOUT_GENERAL,&clearValue, 1,&imageSubresourceRange);
        }
        vkCmdClearColorImage(currentBuffer, emissiveStoreImage->textureImage, VK_IMAGE_LAYOUT_GENERAL,&clearValue, 1,&imageSubresourceRange);

//        std::cout<<"Samples: "<<pc.currentAccumulatedFrame<<"\n";

        const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
		raygenShaderSbtEntry.deviceAddress = getBufferDeviceAddress(raygenShaderBindingTable.buffer);
		raygenShaderSbtEntry.stride = handleSizeAligned;
		raygenShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
		missShaderSbtEntry.deviceAddress = getBufferDeviceAddress(missShaderBindingTable.buffer);
		missShaderSbtEntry.stride = handleSizeAligned;
		missShaderSbtEntry.size = handleSizeAligned * 2;

		VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
		hitShaderSbtEntry.deviceAddress = getBufferDeviceAddress(hitShaderBindingTable.buffer);
		hitShaderSbtEntry.stride = handleSizeAligned;
		hitShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

		/*
			Dispatch the ray tracing commands
		*/
		vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
		vkCmdBindDescriptorSets(currentBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet, 0, 0);
        
        
		vkCmdTraceRaysKHR(
			currentBuffer,
			&raygenShaderSbtEntry,
			&missShaderSbtEntry,
			&hitShaderSbtEntry,
			&callableShaderSbtEntry,
			myRenderer.swapChain->width(),
			myRenderer.swapChain->height(),
			1);

        vkCmdPushConstants(
                currentBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_RAYGEN_BIT_KHR,
                0,
                sizeof(PushConstantBlock_RS),
                &pc
        );

		UpdateUniformbuffers();
        pc.currentAccumulatedFrame++;
	}

	void RayTracing_RS::Create_RT_RenderSystem()
	{
		// Get ray tracing pipeline properties, which will be used later on in the sample
		rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		VkPhysicalDeviceProperties2 deviceProperties2{};
		deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		deviceProperties2.pNext = &rayTracingPipelineProperties;
		vkGetPhysicalDeviceProperties2(myDevice.physicalDevice, &deviceProperties2);

		// Get acceleration structure properties, which will be used later on in the sample
		accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &accelerationStructureFeatures;
		vkGetPhysicalDeviceFeatures2(myDevice.physicalDevice, &deviceFeatures2);
		LoadFunctionsPointers();
		//CreateTopLevelAccelerationStructure(topLevelObjBase);
		CreateStorageImages();
		CreateUniformBuffer();
		CreateRTPipeline();
		CreateShaderBindingTable();
		CreateDescriptorSets();
		readyToDraw = true;


	}

	void RayTracing_RS::UpdateUniformbuffers()
	{
		uniformData.projInverse = glm::inverse(cam.matrices.perspective);
		uniformData.viewInverse = glm::inverse(cam.matrices.view);

		memcpy(ubo.mapped, &uniformData,sizeof(UniformData));
		memcpy(lightBuffer.mapped, &light,sizeof(Light));
	}

	void RayTracing_RS::SetupBottomLevelObj(std::shared_ptr<ModelData> modelData)
	{
		//ModelData combinedMesh2=modelLoader.GetModelVertexAndIndicesTinyObject("C:/Users/carlo/Downloads/VikingRoom.fbx");
//		combinedMesh.CreateAllTextures(myRenderer.GetSwapchain(), ModelHandler::GetInstance()->allTexturesOffset);
		modelsOnScene.push_back(modelData);
		glm::vec3 positions[3];
		glm::vec3 rots[3];
		glm::vec3 scales[3];
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<>dis(0, 4);
		topLevelObjBase.pos = glm::vec3(0.0f);
		topLevelObjBase.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        topLevelObjBase.rot = glm::vec3(1.0f, 1.0f, 1.0f);
		topLevelObjBase.UpdateMatrix();
		ModelHandler::GetInstance()->AddTLAS(topLevelObjBase);
		for (int i=0; i<1; i++)
		{
			float randomPos = dis(gen);
			positions[i] = glm::vec3(.0f, i, i);
			rots[i] = glm::vec3(0);
			scales[i] = glm::vec3(1.0f);
			for (int j = 0; j < modelsOnScene.size(); ++j)
			{
				ModelHandler::GetInstance()->CreateBLAS(positions[i], rots[i], scales[i],*modelsOnScene[j], topLevelObjBase);
			}
		}


	}

    void RayTracing_RS::ResetAccumulatedFrames() {
         pc.currentAccumulatedFrame= 0;
         
    }


}

