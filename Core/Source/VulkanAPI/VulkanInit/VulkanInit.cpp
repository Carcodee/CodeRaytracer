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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(widht, height, appName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
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
		myInstance = instance;

	}
	VulkanInit::~VulkanInit()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void VulkanInit::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto myWindow = reinterpret_cast<VulkanInit*>(glfwGetWindowUserPointer(window));
		myWindow->framebufferResized = true;
		myWindow->widht = width;
		myWindow->height = height;

	}
 }


