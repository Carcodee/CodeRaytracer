#include "Camera.h"

#include <iomanip>
#include <iostream>
#include <set>

#include "VulkanAPI/Utility/InputSystem/InputHandler.h"

namespace VULKAN 
{
	
	Camera::Camera(glm::vec3 camPos)
	{

		position = camPos;
		currentMode = CameraMode::E_Fixed;
		//rot = glm::vec3(0.0f);

		//matrices.rotation=glm::rotate(matrices.rotation, glm::radians(rot.x), glm::vec3(0.0f, 0.0f, 1.0f));
		//matrices.rotation=glm::rotate(matrices.rotation, glm::radians(rot.y), glm::vec3(0.0f, 0.0f, 1.0f));
		//matrices.rotation=glm::rotate(matrices.rotation, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void Camera::UpdateCamera()
	{
		if (currentMode==CameraMode::E_Fixed)
		{
			SetLookAt(lookAt);
		}
		if (currentMode==CameraMode::E_Free)
		{

			SetPosition();
		}
	}

	void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
	{

		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}

	void Camera::SetLookAt(glm::vec3 targetPosition)
	{
		this->lookAt= targetPosition;
		SetPosition();
		matrices.view = glm::lookAt(position,
			this->lookAt,
			glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void Camera::SetPosition()
	{
		
		matrices.view = glm::lookAt(position, position+forward, up);

		//matrices.view = glm::translate(glm::mat4(1.0f), position);
	}

	void Camera::Move(float deltaTime)
	{

		InputHandler * instanceSingleton= InputHandler::GetInstance();
        bool isClicking=(instanceSingleton->GetUserInput(InputHandler::BUTTON_MOUSE0, InputHandler::ACTION_HOLD) ||
                         instanceSingleton->GetUserInput(InputHandler::BUTTON_MOUSE1, InputHandler::ACTION_HOLD));
		if ( isClicking && instanceSingleton->isMouseInsideViewport)
		{
            instanceSingleton->DisableMouse(false);
			RotateCamera();
		}
		else
		{
            instanceSingleton->DisableMouse(true);
			firstMouse = true;
		}

		float inputX = instanceSingleton->GetCutomInput(InputHandler::x);
		float inputY = instanceSingleton->GetCutomInput(InputHandler::y);
		glm::vec3 lastPos = position;
		position += forward * inputY * movementSpeed * deltaTime;
		position += right * inputX  * movementSpeed * deltaTime;

		UpdateCamera();
	}

	void Camera::RotateCamera()
	{

		InputHandler * instanceSingleton= InputHandler::GetInstance();
		glm::vec2 input = instanceSingleton->GetMousePos();

		float currentX = input.x;
		float currentY = input.y;

		if (firstMouse)
		{
			lastX = currentX;
			lastY = currentY;
			firstMouse = false;
		}

		float xOffset =  lastX - currentX;
		float yOffset =  lastY - currentY;

		lastX = currentX;
		lastY = currentY;

		xOffset *= sens;
		yOffset *= sens;
		yaw += xOffset;
		pitch += yOffset;

		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		if (inverseY)
		{
			WorldUp = glm::vec3(0.0f, -1.0f,0.0f);
		}

		glm::vec3 camForward;

		camForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		camForward.y = sin(glm::radians(pitch));
		camForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		forward = normalize(camForward);
		right = normalize(cross(forward, WorldUp));
		up = normalize(cross(right, forward));


	}
}
