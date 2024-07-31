#pragma once
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <GLFW/glfw3.h>


#include <glm/vec2.hpp>

#include "GLFW/glfw3native.h"

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
			KEY_DEL,
			KEY_LEFT_CONTROL,
			KEY_RIGHT_CONTROL
		};

		enum USER_BUTTON
		{
			BUTTON_NONE,
			BUTTON_MOUSE0,
			BUTTON_MOUSE1
		};

		enum INPUT_ACTION 
		{
			ACTION_NONE = 0,
			ACTION_DOWN = 1,
			ACTION_HOLD = 2,
			ACTION_RELEASE = 3
		};


		enum CUSTOM_INPUT 
		{
			x,
			y
		};

		struct InputState
		{
			INPUT_ACTION currentAction = ACTION_NONE;
			USER_KEY currentKey = KEY_NONE;
			bool keyDownDone = false;
		};



		static bool GetUserInput(USER_KEY key, INPUT_ACTION action);
		static bool GetUserInput(USER_BUTTON key, INPUT_ACTION action);

        static void CheckMouse();
		static float GetCutomInput (CUSTOM_INPUT inputType);
		static void UpdateInputStates();
        void DisableMouse(bool value);

		glm::vec2 GetMousePos();
		glm::vec2 GetMouseInput();

		static std::map<USER_KEY, INPUT_ACTION> keysActioned;
		static std::map<USER_BUTTON, INPUT_ACTION> buttonsActioned;
		bool isMouseInsideViewport;
        static bool movingMouse;
	protected:

		static float xInput;
		static float yInput;
		static double xMousePos;
		static double yMousePos;
		static float xMouseInput;
		static float yMouseInput;
        static double lastXPos;
        static double lastYPos;

		InputHandler();
        bool mouseInsideViewport = false;
		InputHandler(InputHandler& other) = delete;
		void operator=(const InputHandler&) = delete;
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static GLFWwindow* userWindow;

		float timeForDeletionKey;


	};
}


