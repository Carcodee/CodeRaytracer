#include "Core.h"
#include "VulkanAPI/VulkanInit.h"

namespace Core {

	void RunEngine(int widht, int height, std::string appName)
	{
		VULKAN::VulkanInit window{ widht, height, appName };
		window.Run();
	}

}