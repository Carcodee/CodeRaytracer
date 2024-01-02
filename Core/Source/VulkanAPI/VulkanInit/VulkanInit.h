#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <string>

#include <direct.h>

namespace VULKAN {

	class VulkanInit
	{
	public: 
		VulkanInit(int w, int h, std::string name);
		void InitWindow();
		VulkanInit(const VulkanInit&) = delete;
		VulkanInit& operator=(const VulkanInit&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(window) ; }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(widht), static_cast<uint32_t>(height) }; }
		void Run();
		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		~VulkanInit();


	private:

		const int widht;
		const int height;
		std::string appName;
		GLFWwindow* window;
		VkInstance myInstance;


	};
}


