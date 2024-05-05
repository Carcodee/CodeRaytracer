#pragma once

#include <tiny_obj_loader.h>

#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/VulkanObjects/Buffers/VKBufferHandler.h"
#include <unordered_map>
#include "VulkanAPI/Utility/Utility.h"

#include <vector>
#include <string>

namespace VULKAN{
	class ModelLoaderHandler
	{

	public:
		ModelLoaderHandler(MyVulkanDevice& device);


		VKBufferHandler* LoadModelTinyObject(std::string path);
		VerticesAndIndices GetModelVertexAndIndicesTinyObject(std::string path);
		MyVulkanDevice& myDevice;


	};
}


