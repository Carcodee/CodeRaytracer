#include "InputHandler.h"

#include <iostream>


namespace VULKAN
{

	InputHandler* InputHandler::instance = nullptr;
    GLFWwindow* InputHandler::userWindow = nullptr;
	InputHandler::USER_KEY InputHandler::userKeyDown=InputHandler::KEY_NONE;
	InputHandler::USER_KEY InputHandler::userKeyHold=InputHandler::KEY_NONE;
	InputHandler::USER_KEY InputHandler::userKeyReleased= InputHandler::KEY_NONE;
	float InputHandler::xInput= 0;
	float InputHandler::yInput= 0;
	float InputHandler::xMousePos= 0;
	float InputHandler::yMousePos= 0;

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

	bool InputHandler::GetUserKeyHold(USER_KEY key)
	{
		if (key== userKeyHold)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool InputHandler::GetUserKeyDown(USER_KEY key)
	{
		if (key== userKeyDown)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool InputHandler::GetUserKeyUp(USER_KEY key)
	{
		if (key == userKeyReleased)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	float InputHandler::GetCutomInput(CUSTOM_INPUT inputType)
	{
		switch (inputType)
		{
		case CUSTOM_INPUT::x:
			return xInput;
		case CUSTOM_INPUT::y:
			return yInput;
		}
		
	}

	glm::vec2 InputHandler::GetMousePos()
	{
		return glm::vec2(xMousePos, yMousePos);
	}

	glm::vec2 InputHandler::GetMouseInput()
	{
	}

	void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{

		userKeyDown = KEY_NONE;
		userKeyHold = KEY_NONE;
		userKeyReleased = KEY_NONE;


		//keydown
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			userKeyDown = KEY_W;
			yInput = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			userKeyDown = KEY_S;
			yInput = -1;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			userKeyDown = KEY_D;
			xInput = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			userKeyDown = KEY_A;
			xInput = -1;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
			userKeyDown = KEY_LSHIFT;
		}

		//keyhold
		if (key == GLFW_KEY_W && action == GLFW_REPEAT) {
			userKeyHold = KEY_W;
			yInput = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_REPEAT) {
			userKeyHold = KEY_S;
			yInput = -1;
		}
		if (key == GLFW_KEY_D && action == GLFW_REPEAT) {
			userKeyHold = KEY_D;
			xInput = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_REPEAT) {
			userKeyHold = KEY_A;
			xInput = -1;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_REPEAT) {
			userKeyHold = KEY_LSHIFT;
		}

		//release


		if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			userKeyHold = KEY_W;
			yInput = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			userKeyHold = KEY_S;
			yInput = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			userKeyHold = KEY_D;
			xInput = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			userKeyHold = KEY_A;
			xInput = 0;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
			userKeyHold = KEY_LSHIFT;
		}



	}

	void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{

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

}

