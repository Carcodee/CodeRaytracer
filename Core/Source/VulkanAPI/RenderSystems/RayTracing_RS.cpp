#include "RayTracing_RS.h"

#include "VulkanAPI/Model/ModelHandler.h"


namespace VULKAN {
	RayTracing_RS::RayTracing_RS(MyVulkanDevice& device, VulkanRenderer& renderer):myDevice(device), myRenderer(renderer)
	{
		cam.SetPerspective(60.0f, (float)800 / (float)600, 0.1f, 512.0f);
		cam.position=(glm::vec3(0.0f, 4.0f, 0.0f));
        cam.SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));
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
	//TODO: Create the storage image of the raytracing
	void RayTracing_RS::CreateStorageImage()
	{

		storageImage = new VKTexture(myRenderer.GetSwapchain(), myRenderer.GetSwapchain().width(), myRenderer.GetSwapchain().height(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_FORMAT_R8G8B8A8_UNORM);
		
	}

	void RayTracing_RS::CreateBottomLevelAccelerationStructureModel(BottomLevelObj& obj)
	{
		indexCount = static_cast<uint32_t>(obj.indices.size());

		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vertexBuffer,
			obj.vertices.size() * sizeof(Vertex),
			obj.vertices.data());
		// Index buffer
		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&indexBuffer,
			obj.indices.size() * sizeof(uint32_t),
			obj.indices.data());
		// Transform buffer
		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&transformBuffer,
			sizeof(VkTransformMatrixKHR),
			&obj.matrix);

		VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

		vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer.buffer);
		indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(indexBuffer.buffer);
		transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer.buffer);

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.maxVertex = 2;
		accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
		accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
		accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
		accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
		accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		const uint32_t numTriangles =indexCount/3;
		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			myDevice.device(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&numTriangles,
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

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = obj.BottomLevelAs.handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
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
		accelerationDeviceAddressInfo.accelerationStructure = obj.BottomLevelAs.handle;
		obj.BottomLevelAs.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(myDevice.device(), &accelerationDeviceAddressInfo);



		DeleteScratchBuffer(scratchBuffer);

	}

	void RayTracing_RS::CreateTopLevelAccelerationStructure(TopLevelObj& topLevelObj)
	{
		std::vector<VkAccelerationStructureInstanceKHR> instaces;
			

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = topLevelObj.matrix;
		instance.instanceCustomIndex = topLevelObj.topLevelInstanceCount;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase, 0).BottomLevelAs.deviceAddress;

		Buffer instancesBuffer;

		myDevice.createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&instancesBuffer,
			sizeof(VkAccelerationStructureInstanceKHR),
			&instance);

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

		uint32_t primitive_count = 1;

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
		myDevice.createBuffer(bufferUsageFlags, memoryUsageFlags, &missShaderBindingTable, handleSize);
		myDevice.createBuffer(bufferUsageFlags, memoryUsageFlags, &hitShaderBindingTable, handleSize);

		// Copy handles
		raygenShaderBindingTable.device = myDevice.device();
		missShaderBindingTable.device = myDevice.device();
		hitShaderBindingTable.device = myDevice.device();

		raygenShaderBindingTable.map();
		missShaderBindingTable.map();
		hitShaderBindingTable.map();
		memcpy(raygenShaderBindingTable.mapped, shaderHandleStorage.data(), handleSize);
		memcpy(missShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize);
		memcpy(hitShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);

	}

	void RayTracing_RS::CreateDescriptorSets()
	{
		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
		};

		VKTexture* texture = new VKTexture("C:/Users/carlo/Downloads/VikkingRoomTextures.png", myRenderer.GetSwapchain());
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = INITIALIZERS::descriptorPoolCreateInfo(poolSizes, 1);
		if (vkCreateDescriptorPool(myDevice.device(), &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create descriptorPools");
		}

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =INITIALIZERS::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

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
		roomText.imageView = texture->textureImageView;
		roomText.sampler = texture->textureSampler;
		roomText.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		ubo.descriptor.buffer = ubo.buffer;
		ubo.descriptor.offset = 0;
		ubo.descriptor.range = sizeof(UniformData);
		 
		vertexBuffer.descriptor.buffer = vertexBuffer.buffer;
		vertexBuffer.descriptor.offset = 0;
		vertexBuffer.descriptor.range = sizeof(Vertex) * ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase,0).vertices.size();

		indexBuffer.descriptor.buffer = indexBuffer.buffer;
		indexBuffer.descriptor.offset = 0;
		indexBuffer.descriptor.range = sizeof(uint32_t) *  ModelHandler::GetInstance()->GetBLASFromTLAS(topLevelObjBase,0).indices.size();


		VkWriteDescriptorSet resultImageWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor);
		VkWriteDescriptorSet uniformBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &ubo.descriptor);
		VkWriteDescriptorSet textWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &roomText);
		VkWriteDescriptorSet vertexBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &vertexBuffer.descriptor);
		VkWriteDescriptorSet indexBufferWrite = INITIALIZERS::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5, &indexBuffer.descriptor);

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			accelerationStructureWrite,
			resultImageWrite,
			uniformBufferWrite,
			textWrite,
			vertexBufferWrite,
			indexBufferWrite
		};

		vkUpdateDescriptorSets(myDevice.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
	}

	void RayTracing_RS::CreateRTPipeline()
	{
		VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
		accelerationStructureLayoutBinding.binding = 0;
		accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		accelerationStructureLayoutBinding.descriptorCount = 1;
		accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

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
		TextureBinding.descriptorCount = 1;
		TextureBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

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



		std::vector<VkDescriptorSetLayoutBinding> bindings({
			accelerationStructureLayoutBinding,
			resultImageLayoutBinding,
			uniformBufferBinding,
			TextureBinding,
			vertexBinding,
			indexBinding
			});

		VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCI{};
		descriptorSetlayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetlayoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
		descriptorSetlayoutCI.pBindings = bindings.data();
		if (vkCreateDescriptorSetLayout(myDevice.device(), &descriptorSetlayoutCI, nullptr, &descriptorSetLayout)!=VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create descriptor set layouts");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
		if (vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create pipeline layout");
		}

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::string path;

		// Ray generation group
		{
			path = "C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/RayTracingShaders/raygen.rgen.spv";
			//path = "../Core/Source/Shaders/RayTracingShaders/raygen.rgen.spv";
			shaderStages.push_back(PipelineReader::CreateShaderStageModule(rGenShaderModule,myDevice,VK_SHADER_STAGE_RAYGEN_BIT_KHR, path));
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
			path = "C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/RayTracingShaders/miss.rmiss.spv";
			//path = "../Core/Source/Shaders/RayTracingShaders/raygen.rgen.spv";
			shaderStages.push_back(PipelineReader::CreateShaderStageModule(rMissShaderModule, myDevice, VK_SHADER_STAGE_MISS_BIT_KHR, path));
			VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			this->shaderGroups.push_back(shaderGroup);
		}

		// Closest hit group
		{
			path = "C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/RayTracingShaders/closesthit.rchit.spv";
			//path = "../Core/Source/Shaders/RayTracingShaders/raygen.rgen.spv";
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
		UpdateUniformbuffers();
	}

	uint32_t RayTracing_RS::GetShaderBindAdress(uint32_t hitGroupStart, uint32_t start, uint32_t offset,
		uint32_t stbRecordOffset, uint32_t geometryIndex, uint32_t stbRecordStride)
	{
		return hitGroupStart + start * (offset + stbRecordOffset + (geometryIndex + stbRecordStride));
	}

	void RayTracing_RS::DrawRT(VkCommandBuffer& currentBuffer)
	{
		if (storageImage->currentLayout!= VK_IMAGE_LAYOUT_GENERAL)
		{
				VkCommandBuffer commandBuffer = myDevice.beginSingleTimeCommands();
			VkImageMemoryBarrier barrier{};

			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = storageImage->textureImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			myDevice.endSingleTimeCommands(commandBuffer);
			storageImage->currentLayout = VK_IMAGE_LAYOUT_GENERAL;

		}
		
		VkImageSubresourceRange imageSubresourceRange = {};
		imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceRange.baseMipLevel = 0;
		imageSubresourceRange.levelCount = 1; 
		imageSubresourceRange.baseArrayLayer = 0;
		imageSubresourceRange.layerCount = 1; 
		const VkClearColorValue clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
		vkCmdClearColorImage(currentBuffer, storageImage->textureImage, VK_IMAGE_LAYOUT_GENERAL,&clearValue, 1,&imageSubresourceRange);

		const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
		raygenShaderSbtEntry.deviceAddress = getBufferDeviceAddress(raygenShaderBindingTable.buffer);
		raygenShaderSbtEntry.stride = handleSizeAligned;
		raygenShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
		missShaderSbtEntry.deviceAddress = getBufferDeviceAddress(missShaderBindingTable.buffer);
		missShaderSbtEntry.stride = handleSizeAligned;
		missShaderSbtEntry.size = handleSizeAligned;

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

		//TODO::
		UpdateUniformbuffers();
	
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


		SetupBottomLevelObj();
		CreateBottomLevelAccelerationStructureModel(ModelHandler::GetInstance()->bottomLevelObjects.at(topLevelObjBase.TLASID)[0]);
		CreateTopLevelAccelerationStructure(topLevelObjBase);

		CreateStorageImage();
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
	}

	void RayTracing_RS::TransitionStorageImage()
	{
		VkCommandBuffer commandBuffer = myDevice.beginSingleTimeCommands();
			VkImageMemoryBarrier barrier{};

			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = storageImage->textureImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			myDevice.endSingleTimeCommands(commandBuffer);
			storageImage->currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	}

	void RayTracing_RS::SetupBottomLevelObj()
	{
		VerticesAndIndices verticesAndIndices=modelLoader.GetModelVertexAndIndicesTinyObject("C:/Users/carlo/Downloads/VikingRoom.fbx");

		glm::vec3 positions[3];
		glm::vec3 rots[3];
		glm::vec3 scales[3];
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<>dis(0, 4);
		topLevelObjBase.pos = glm::vec3(0.0f);
		topLevelObjBase.scale = glm::vec3(1.0f, 1.0f, -1.0f);
		topLevelObjBase.UpdateMatrix();
		ModelHandler::GetInstance()->AddTLAS(topLevelObjBase);
		for (int i=0; i<3; i++)
		{
			float randomPos = dis(gen);
			positions[i] = glm::vec3(0);
			rots[i] = glm::vec3(rand() / RAND_MAX);
			scales[i] = glm::vec3(1.0f);
			ModelHandler::GetInstance()->CreateBLAS(positions[i], rots[i], scales[i],verticesAndIndices.vertices, verticesAndIndices.indices, topLevelObjBase);
		}

	}
}

