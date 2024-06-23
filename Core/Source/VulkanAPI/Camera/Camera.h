#pragma once
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace VULKAN 
{
	enum CameraMode
	{
		E_Fixed=0,
		E_Free =1
	};
	class Camera
	{
	public:
		struct matrices
		{
			glm::mat4 perspective;
			glm::mat4 view;
			glm::mat4 rotation;
			glm::mat4 translation;
		}matrices;
	

		Camera(glm::vec3 camPos);

		void UpdateCamera();
		void SetPerspective(float fov, float aspect, float znear, float zfar);
		void SetLookAt(glm::vec3 targetPosition);	
		void SetPosition();	
		void Move(float deltaTime);
		void RotateCamera();

		
		glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		glm::vec3 position;

		glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 right;
		glm::vec3 up;
		glm::vec3 lookAt = glm::vec3(0.0f);

		glm::vec3 rotX = glm::vec3(0.0f);
		glm::vec3 rotZ = glm::vec3(0.0f);
		glm::vec3 rotY = glm::vec3(0.0f);

		float yaw = -90;
		float pitch = -90;
		float lastX;
		float lastY;
		float sens = .4f;
		bool firstMouse = true;
		bool inverseY = true;
		CameraMode currentMode = CameraMode::E_Fixed;
		float movementSpeed = 5.0f;

	};

}

