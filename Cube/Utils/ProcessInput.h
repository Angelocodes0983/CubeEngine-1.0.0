#pragma once
#include "Dependencies.h"
#include "Camera.h"
#include "Renderer.h"
#include "Input.h"
#include "Destruction.h"
#include "Ui.h"
#include "Raycasting.h"

namespace Cube
{
	static inline bool defualtCameraMovement = true;
	static inline bool mouseCaptured = true;
	static inline bool oKeyPressed = false;

	inline bool use = true;
	inline void toggleMouse() {
		mouseCaptured = !mouseCaptured;

		if (mouseCaptured) {
			glfwSetInputMode(Renderer::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(Renderer::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	inline void processInput()
	{
		if (!defualtCameraMovement) { return; }

		
		if (Input::IsKeyPressed(Key::W))
			Renderer::camera.processKeyboard(GLFW_KEY_W, deltaTime);
		if (Input::IsKeyPressed(Key::S))
			Renderer::camera.processKeyboard(GLFW_KEY_S, deltaTime);
		if (Input::IsKeyPressed(Key::A))
			Renderer::camera.processKeyboard(GLFW_KEY_A, deltaTime);
		if (Input::IsKeyPressed(Key::D))
			Renderer::camera.processKeyboard(GLFW_KEY_D, deltaTime);
		if (Input::IsKeyPressed(Key::Space))
			Renderer::camera.processKeyboard(GLFW_KEY_SPACE, deltaTime);
		if (Input::IsKeyPressed(Key::LeftShift))
			Renderer::camera.processKeyboard(GLFW_KEY_LEFT_SHIFT, deltaTime);
		if (Input::IsKeyPressed(Key::LeftControl))
			Renderer::camera.processKeyboard(GLFW_KEY_LEFT_CONTROL, deltaTime);
		if (Input::IsKeyPressed(Key::Q))
			Renderer::camera.processKeyboard(GLFW_KEY_Q, deltaTime);

		if (Input::IsKeyPressed(Key::G))
		{
			if (use)
			{
				int id = 1;
				floodFill(*ObjectManager::physicsIdToVoxelData.at(id), 99, ObjectManager::physicsIdToGridSize[id], id);
				Renderer::createVoxelTexture(id, *ObjectManager::physicsIdToVoxelData.at(id), ObjectManager::physicsIdToGridSize[id].x, ObjectManager::physicsIdToGridSize[id].y, ObjectManager::physicsIdToGridSize[id].z);

				std::cout << "created tex" << std::endl;
				Renderer::updateVoxelObjectDescriptor();
				std::cout << "updated descriptor" << std::endl;

				Bricks::updateBricks(*ObjectManager::physicsIdToVoxelData.at(id), ObjectManager::physicsIdToGridSize[id], glm::vec3(0), id);
				std::cout << "updated bricks" << std::endl;

				Renderer::updateBrickBuffer();

				Renderer::updateBrickElementsBuffer();
		
				//Renderer::createVoxelTexture(1, *ObjectManager::physicsIdToVoxelData.at(id), ObjectManager::physicsIdToGridSize[id].x,
				///	ObjectManager::physicsIdToGridSize[id].y, ObjectManager::physicsIdToGridSize[id].z);
				//Renderer::updateVoxelObjectDescriptor();
				use = false;
			}
		}
	
		static bool previousClick = false;
		bool leftMouse = glfwGetMouseButton(Renderer::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		if (leftMouse && !previousClick)
		{
			size_t id = raycastFromMouseForPhysicsId();
			glm::vec3 localPos = voxelRayCast(id, *ObjectManager::physicsIdToVoxelData.at(id));
			Cube::destructionSphere(id, localPos, DESTRUCTIONRADIUS);
		}
		previousClick = leftMouse;


		if (Input::IsKeyPressed(Key::O)) {
			if (!oKeyPressed) {
				toggleMouse();
				oKeyPressed = true;
			}
		}
		else {
			oKeyPressed = false;
		}
	}
}