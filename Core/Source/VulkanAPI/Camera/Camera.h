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
		
		glm::vec3 position;
		glm::vec3 lookAt = glm::vec3(0.0f);

		glm::vec3 rot = glm::vec3(0.0f);

		CameraMode currentMode = CameraMode::E_Fixed;

		float rotationSpeed = 1.0f;
		float movementSpeed = 1.0f;
	private:
	};

}

