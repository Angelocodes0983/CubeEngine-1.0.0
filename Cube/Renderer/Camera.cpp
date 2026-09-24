#include "Camera.h"

namespace Cube
{

	void Camera::updateCameraVectors() {
		glm::vec3 newFront;
		newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newFront.y = sin(glm::radians(pitch));
		newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(newFront);

		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}

	Camera::Camera(glm::vec3 startPosition,
		glm::vec3 startUp,
		float startYaw,
		float startPitch)
		: front(glm::vec3(0.0f, 0.0f, -1.0f)),
		movementSpeed(23.5f),
		mouseSensitivity(20.0f),
		zoom(45.0f) {
		position = startPosition;
		worldUp = startUp;
		yaw = startYaw;
		pitch = startPitch;
		updateCameraVectors();
	}


	glm::mat4 Camera::getViewMatrix() {
		glm::mat4 view = glm::lookAt(glm::vec3(position), glm::vec3(position) + front, up);

		return view;
	}

	glm::mat4 Camera::getProjectionMatrix(float aspectRatio) {
		glm::mat4 proj = glm::perspective(glm::radians(zoom), aspectRatio, 0.1f, 100.0f);

		return proj;
	}


	void Camera::processKeyboard(int key, float deltaTime) {
		float velocity = movementSpeed * deltaTime;


		if (key == GLFW_KEY_W)
			position += front * velocity;
		if (key == GLFW_KEY_S)
			position -= front * velocity;
		if (key == GLFW_KEY_A)
			position -= right * velocity;
		if (key == GLFW_KEY_D)
			position += right * velocity;
		if (key == GLFW_KEY_SPACE)
			position += worldUp * velocity;
		if (key == GLFW_KEY_LEFT_SHIFT)
			position -= worldUp * velocity;
		if (key == GLFW_KEY_LEFT_CONTROL)
			movementSpeed += 1;
		if (key == GLFW_KEY_Q)
			movementSpeed = 23;

	}

	void Camera::processMouseMovement(float xoffset, float yoffset, float deltaTime, bool constrainPitch ) {

		xoffset = mouseSensitivity * xoffset * deltaTime;
		yoffset = mouseSensitivity * yoffset * deltaTime;

		yaw += xoffset;
		pitch -= yoffset;

		if (constrainPitch) {
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		updateCameraVectors();
	}

	void Camera::processMouseScroll(float yoffset) {
		zoom -= yoffset;
		if (zoom < 1.0f)
			zoom = 1.0f;
		if (zoom > 45.0f)
			zoom = 45.0f;
	}

	glm::vec3 Camera::getPosition() const { return position; }
	glm::dvec3 Camera::getPositionPrecise() const { return position; }
	void Camera::setPosition(glm::vec3 pos)
	{
		position = pos;
	}
	glm::vec3 Camera::getFront() const { return front; }
	glm::vec3 Camera::getRight() const { return right; }
	void Camera::setFront(glm::vec3 vec)
	{
		front = vec;
	}
	void Camera::setRight(glm::vec3 vec)
	{
		right = vec;
	}
	void Camera::setUp(glm::vec3 vec)
	{
		up = vec;
	}
	float Camera::getZoom() const { return zoom; }
	float Camera::getYaw() const { return yaw; }
	float Camera::getPitch() const { return pitch; }

	void Camera::processMouse(double xpos, double ypos) {
		if (!shouldProcessMouse) { return; }

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = ypos - lastY;

		lastX = xpos;
		lastY = ypos;

		processMouseMovement(xoffset, yoffset, deltaTime);
	}


}