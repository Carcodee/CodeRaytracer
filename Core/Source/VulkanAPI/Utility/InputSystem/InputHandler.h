#pragma once
#include <string>
#include <GLFW/glfw3.h>


#include <glm/vec2.hpp>

namespace VULKAN
{
	class InputHandler
	{
		static InputHandler* instance;

	public:
		static InputHandler* GetInstance(GLFWwindow* window= nullptr);

		enum USER_KEY
		{
			KEY_NONE,
			KEY_Q,
			KEY_W,
			KEY_E,
			KEY_R,
			KEY_A,
			KEY_S,
			KEY_D,
			KEY_LSHIFT,
			BUTTON_MOUSE0,
			BUTTON_MOUSE1
		};

		enum CUSTOM_INPUT 
		{
			x,
			y
		};
		bool GetUserKeyHold(USER_KEY key);
		bool GetUserKeyDown(USER_KEY key);
		bool GetUserKeyUp(USER_KEY key);
		float GetCutomInput (CUSTOM_INPUT inputType);
		glm::vec2 GetMousePos();
		glm::vec2 GetMouseInput();

	protected:


		static USER_KEY userKeyDown;
		static USER_KEY userKeyHold;
		static USER_KEY userKeyReleased;
		static float xInput;
		static float yInput;
		static float xMousePos;
		static float yMousePos;
		static float xMouseInput;
		static float yMouseInput;

		InputHandler();
		InputHandler(InputHandler& other) = delete;
		void operator=(const InputHandler&) = delete;
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static GLFWwindow* userWindow;


	};
}


