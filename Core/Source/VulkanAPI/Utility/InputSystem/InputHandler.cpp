#include "InputHandler.h"

#include <iostream>


namespace VULKAN
{

	InputHandler* InputHandler::instance = nullptr;
    GLFWwindow* InputHandler::userWindow = nullptr;
	std::map<InputHandler::USER_KEY, InputHandler::INPUT_ACTION> InputHandler::keysActioned;
	std::map<InputHandler::USER_BUTTON, InputHandler::INPUT_ACTION> InputHandler::buttonsActioned;

	float InputHandler::xInput= 0;
	float InputHandler::yInput= 0;
	float InputHandler::xMousePos= 0;
	float InputHandler::yMousePos= 0;
	float InputHandler::xMouseInput= 0;
	float InputHandler::yMouseInput= 0;

	InputHandler::InputHandler()
	{
	}

	InputHandler* InputHandler::GetInstance(GLFWwindow* window)
	{
		if (instance==nullptr)
		{
			instance = new InputHandler();
			if (window!=nullptr)
			{
				userWindow = window;
				glfwSetKeyCallback(window, keyCallback);
				glfwSetMouseButtonCallback(window, mouseButtonCallback);
				glfwSetScrollCallback(window, scrollCallback);
				glfwSetCursorPosCallback(window, cursorPositionCallback);


			}
		}
		return instance;
	}

	bool InputHandler::GetUserInput(USER_KEY key, INPUT_ACTION action)
	{

		if (keysActioned.contains(key))
		{
			if (keysActioned.at(key) == action)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}

	}

	bool InputHandler::GetUserInput(USER_BUTTON key, INPUT_ACTION action)
	{

		if (buttonsActioned.contains(key))
		{
			if (buttonsActioned.at(key) == action)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}


	}


	float InputHandler::GetCutomInput(CUSTOM_INPUT inputType)
	{
		bool movingX = false;
		bool movingY = false;
		switch (inputType)
		{
		case CUSTOM_INPUT::x:
			if (GetUserInput(KEY_A, ACTION_HOLD)||GetUserInput(KEY_A, ACTION_DOWN) )
			{
				xInput = -1.0f;
				movingX = true;
			}
			if (GetUserInput(KEY_D, ACTION_HOLD)||GetUserInput(KEY_D, ACTION_DOWN) )
			{
				xInput = 1.0f;
				movingX = true;
			}
			if (!movingX)
			{
				xInput = 0.0f;
			}
			return xInput;

		case CUSTOM_INPUT::y:

			if (GetUserInput(KEY_W, ACTION_HOLD) || GetUserInput(KEY_W, ACTION_DOWN))
			{
				yInput = 1.0f;
				movingY = true;
			}
			if (GetUserInput(KEY_S, ACTION_HOLD) || GetUserInput(KEY_S, ACTION_DOWN))
			{
				yInput = -1.0f;
				movingY = true;
			}
			if (!movingY)
			{
				yInput = 0.0f;
			}

			return yInput;
		}
		
	}

	glm::vec2 InputHandler::GetMousePos()
	{
		return glm::vec2(xMousePos, yMousePos);
	}

	glm::vec2 InputHandler::GetMouseInput()
	{
		return glm::vec2(xMouseInput, yMouseInput);
	}

	void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{

		USER_KEY currentKey = KEY_NONE;
		INPUT_ACTION currentAction = ACTION_NONE;


		//keydown
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			currentKey = KEY_W;
			currentAction = ACTION_DOWN;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			currentKey = KEY_S;
			currentAction = ACTION_DOWN;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			currentKey = KEY_D;
			currentAction = ACTION_DOWN;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			currentKey = KEY_A;
			currentAction = ACTION_DOWN;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
			currentKey = KEY_LSHIFT;
			currentAction = ACTION_DOWN;
		}

		//release
		if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			currentKey = KEY_W;
			currentAction = ACTION_RELEASE;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			currentKey = KEY_S;
			currentAction = ACTION_RELEASE;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			currentKey = KEY_D;
			currentAction = ACTION_RELEASE;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			currentKey = KEY_A;
			currentAction = ACTION_RELEASE;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
			currentKey = KEY_LSHIFT;
			currentAction = ACTION_RELEASE;
		}

		if (keysActioned.contains(currentKey))
		{
			keysActioned.at(currentKey) = currentAction;
		}
		else
		{
			keysActioned.try_emplace(currentKey, currentAction);
		}
		

	}

	void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{


		USER_BUTTON currentButton = BUTTON_NONE;
		INPUT_ACTION currentActionMade = ACTION_NONE;

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			currentButton = BUTTON_MOUSE0;
			currentActionMade = ACTION_DOWN;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{
			currentButton = BUTTON_MOUSE1;
			currentActionMade = ACTION_DOWN;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{
			currentButton = BUTTON_MOUSE0;
			currentActionMade = ACTION_RELEASE;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		{
			currentButton = BUTTON_MOUSE1;
			currentActionMade = ACTION_RELEASE;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		if (buttonsActioned.contains(currentButton))
		{
			buttonsActioned.at(currentButton) = currentActionMade;
		}
		else
		{
			buttonsActioned.try_emplace(currentButton, currentActionMade);
		}

	}
	void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if (xpos!=xMousePos)
		{
			xMousePos = xpos;
			xMouseInput = 1.0f;
		}
		else
		{
			xMouseInput = 0.0f;
		}
		if (ypos!=yMousePos)
		{
			yMousePos = ypos;
			yMouseInput = 1.0f;
		}
		else
		{
			yMouseInput = 0.0f;
		}
	}

	void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
	}

	void InputHandler::UpdateInputStates()
	{

		for (auto& key : keysActioned)
		{
			if (key.second== ACTION_RELEASE)
			{
				key.second = ACTION_NONE;
			}
			if (key.second== ACTION_DOWN)
			{
				key.second = ACTION_HOLD;
			}
		}
		for (auto& key : buttonsActioned)
		{
			if (key.second== ACTION_RELEASE)
			{
				key.second = ACTION_NONE;
			}
			if (key.second== ACTION_DOWN)
			{
				key.second = ACTION_HOLD;
			}
		}



	}
}

