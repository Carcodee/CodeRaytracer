#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>

#include <direct.h>
#include "functional"
//#include "VulkanAPI/Headers/HeadersFiles.h"
#include <deque>

#define PRINTLVK(x) std::cout << #x << std::endl;

namespace VULKAN {

	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void push_function(std::function<void()>&& function) {
			deletors.push_back(function);
		}

		void flush() {
			// reverse iterate the deletion queue to execute all the functions
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
				(*it)(); //call the function
			}

			deletors.clear();
		}
	};
	class VulkanInit
	{
	public: 
		GLFWwindow* window;
		VulkanInit(int w, int h, std::string name);
		void InitWindow();
		VulkanInit(const VulkanInit&) = delete;
		VulkanInit& operator=(const VulkanInit&) = delete;

		VkExtent2D getExtent() { return { static_cast<uint32_t>(widht), static_cast<uint32_t>(height) }; }
		bool ShouldClose() { return glfwWindowShouldClose(window); }
		bool WasWindowResized() { return framebufferResized; }
		void ResetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* GetWindow() { return window; }

		void Run();
		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		~VulkanInit();



	private:

		int widht;
		int height;

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void dropCallback(GLFWwindow* window, int count, const char** paths);
		bool framebufferResized = false;

		std::string appName;
		VkInstance myInstance;


	};
}


