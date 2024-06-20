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
		
		matrices.view = glm::translate(glm::mat4(1.0f), position);
	}

	void Camera::Move(float x, float y,float z, float deltaTime)
	{


		glm::vec3 dir(x, y, z);
		glm::vec3 lastPos = position;
		position += dir * movementSpeed * deltaTime;
		//std::cout << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";

		UpdateCamera();
	}

	void Camera::RotateCamera(float deltaTime)
	{
		InputHandler * instanceSingleton= InputHandler::GetInstance();
		glm::vec2 input = instanceSingleton->GetMouseInput();
		currentForwardAngle += input * rotationSpeed * deltaTime;

	}
}
