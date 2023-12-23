#include "VulkanInit.h"


namespace VULKAN {

	VulkanInit::VulkanInit(int w, int h, std::string name) : widht{w}, height{h}, appName{name}
	{
		InitWindow();
	}

	void VulkanInit::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(widht, height, appName.c_str(), nullptr, nullptr);
	}

	void VulkanInit::Run()
	{
		while (!glfwWindowShouldClose)
		{
			glfwPollEvents();
		}
	}
	VulkanInit::~VulkanInit()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
 }


