#include "Utility.h"

namespace VULKAN {


	uint32_t alignedSize(uint32_t value, uint32_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	size_t alignedSize(size_t value, size_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	VkDeviceSize alignedVkSize(VkDeviceSize value, VkDeviceSize alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}


}