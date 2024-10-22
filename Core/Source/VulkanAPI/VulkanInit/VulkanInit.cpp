#include "VulkanInit.h"
#include "FileSystem/FileHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"

#include <stdexcept>

//#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#include "VulkanAPI/Model/ModelHandler.h"
#include "VulkanAPI/Utility/InputSystem/InputHandler.h"

namespace VULKAN {

	VulkanInit::VulkanInit(int w, int h, std::string name) : widht{w}, height{h}, appName{name}
	{
		InitWindow();

	}

	void VulkanInit::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(widht, height, appName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		glfwSetDropCallback(window, dropCallback);
		if (window!=nullptr)
		{
			std::cout << "GLFW initialized" << std::endl;
		}
		InputHandler* instanceSingleton = InputHandler::GetInstance(window);


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

	void VulkanInit::dropCallback(GLFWwindow* window, int count, const char** paths)
	{
        
		for (int i = 0; i < count; ++i)
		{
            std::string path = std::string(paths[i]);
            if (AssetsHandler::GetInstance()->IsValidImageFormat(HELPERS::FileHandler::GetInstance()->GetPathExtension(path))){
                ModelHandler::GetInstance()->AddTexture(path);
                continue;
            }
			ModelHandler::GetInstance()->AddModelToQuery(path);
		}

	}
}


