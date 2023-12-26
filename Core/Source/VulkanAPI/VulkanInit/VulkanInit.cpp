#include "VulkanInit.h"

#include <stdexcept>

namespace VULKAN {

	VulkanInit::VulkanInit(int w, int h, std::string name) : widht{w}, height{h}, appName{name}
	{
		InitWindow();

	}

	void VulkanInit::InitWindow()
	{
		char buffer[_MAX_PATH];
		if (_getcwd(buffer, sizeof(buffer)) != NULL) {
			std::cout << "Current Working Directory: " << buffer << std::endl;
		}
		else {
			std::cerr << "Error getting current working directory" << std::endl;
		}
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(widht, height, appName.c_str(), nullptr, nullptr);
	}

	void VulkanInit::Run()
	{
		while (!ShouldClose())
		{
			glfwPollEvents();
		}
	}
	void VulkanInit::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface)!=VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface");
		}

	}
	VulkanInit::~VulkanInit()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
 }


