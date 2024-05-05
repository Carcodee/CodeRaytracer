#include "Camera.h"

#include <set>

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
	}

	void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
	{

		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}

	void Camera::SetLookAt(glm::vec3 targetPosition)
	{
		this->lookAt= targetPosition;
		matrices.view = glm::lookAt(position,
			this->lookAt,
			glm::vec3(0.0f, 0.0f, 1.0f));
	}
}
