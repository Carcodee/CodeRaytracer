#include "Core.h"
#include "VulkanApp.h"
namespace Core {

	void RunEngine(int widht, int height, std::string appName)
	{
		VULKAN::VulkanApp app(true, false);
		app.Run();
		
	}

}