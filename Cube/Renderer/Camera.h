#pragma once
#include "Dependencies.h"
#include "Time.h"



namespace Cube
{

	struct CameraData {
		glm::mat4 view;              // 64 bytes
		glm::mat4 projection;        // 64 bytes
		glm::vec3 cameraPos;         // 12 bytes
		float padding1;              // 4 bytes (pad to 16)
		int frameCount;              // 4 bytes
		float time;                  // 4 bytes
		float padding2[2];           // 8 bytes (pad to 16)
		glm::vec3 worldMin;          // 12 bytes
		float padding3;              // 4 bytes (pad to 16)
		glm::ivec3 worldMax;          // 12 bytes
		float voxelObjectCount;              // 4 bytes (pad to 16)
		glm::ivec3 gridSize;         // 12 bytes
		float selectedObject;              // 4 bytes (pad to 16)
		glm::mat4 invViewProj;        // 64 bytes


		CameraData() {
			padding1 = 0.0f;
			padding2[0] = padding2[1] = 0.0f;
			padding3 = 0.0f;

		}
	};

	class Camera
	{
	public:

		Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
			float startYaw = -90.0f,
			float startPitch = 0.0f);

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix(float aspectRatio);
		void processKeyboard(int key, float deltaTime);

		void processMouse(double xpos, double ypos);

		void processMouseMovement(float xoffset, float yoffset, float deltaTime, bool constrainPitch = true);
		void processMouseScroll(float yoffset);

		glm::vec3 getPosition() const;
		glm::dvec3 getPositionPrecise() const;
		void setPosition(glm::vec3 pos);

		glm::vec3 getFront() const;
		glm::vec3 getRight() const;

		void setFront(glm::vec3 vec);
		void setRight(glm::vec3 vec);
		void setUp(glm::vec3 vec);

		float getZoom() const;
		float getYaw() const;
		float getPitch() const;


	private:

		bool shouldProcessMouse = true;

		bool firstMouse = true;
		float lastX = 0.0f;
		float lastY = 0.0f;

		glm::dvec3 position;
		glm::vec3 front;
		glm::vec3 up;
		glm::vec3 right;
		glm::vec3 worldUp;

		float yaw;
		float pitch;
		float movementSpeed;
		float mouseSensitivity;
		float zoom;

		void updateCameraVectors();
	
	};
}