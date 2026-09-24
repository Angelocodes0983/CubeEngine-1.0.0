#pragma once
#include "Cube.h"


namespace Cube
{
	
	inline bool isHold = false;
	inline bool leftMouseWasPressed = false;
	inline bool rightMouseWasPressed = false;
	inline size_t heldPhysicsId = 0;
	inline float holdDistance = 80.0f;

	inline void createTestSetup()
	{
		std::string cube32 = (Renderer::current_path / "assets/defualtShape32.vox").string();
		Physics::phyGravityScale(9.81f);
		Cube::ObjectManager::addVoxelObjectFromFile(
			cube32,
			glm::vec3(0, 200,0),
			glm::quat(1, 1, 1, 1),
			glm::vec3(1.0f),
			true,
			glm::vec3(16),
			0
		);
		Cube::ObjectManager::addVoxelObjectFromFile(
			cube32,
			glm::vec3(0, 300, 0),
			glm::quat(1, 1, 1, 1),
			glm::vec3(1.0f),
			true,
			glm::vec3(16),
			0
		);
		Cube::ObjectManager::addVoxelObjectFromFile(
			(Renderer::current_path / "assets/minecraft.vox").string(),
			glm::vec3(0, 0, 0),
			glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 0.0f)),
			glm::vec3(1.0f),
			false,
			glm::vec3(256),
			1
		);
		Renderer::updateBrickBuffer();
		Renderer::updateBrickElementsBuffer();
		Renderer::updateVoxelObjectDescriptor();
			
	}


	inline void appInput()
	{
		bool rightDown = glfwGetMouseButton(Renderer::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
		if (rightDown && !rightMouseWasPressed) {
			if (!isHold) {


				size_t id = raycastForPhysicsId(5000.0f);
				std::cout << "raycasting for id" << std::endl;
				if (id != 0 && id != SIZE_MAX) {
					std::cout << "id" << id<< std::endl;

					btRigidBody* body = Physics::GetBodyById(id);
					if (body && body->getInvMass() > 0.0f) {
						heldPhysicsId = id;
						isHold = true;
						std::cout << "Picked up physics ID: " << heldPhysicsId << "\n";
					}
				}
			}
			else {
				isHold = false;
				heldPhysicsId = 0;
			}
		}
		rightMouseWasPressed = rightDown;

		bool leftDown = glfwGetMouseButton(Renderer::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		if (leftDown && !leftMouseWasPressed) {
			if (isHold && heldPhysicsId != 0) {
				btRigidBody* body = Physics::GetBodyById(heldPhysicsId);
				glm::vec3 launchDir = glm::normalize(Renderer::camera.getFront());

				float force = 400.0f;
				float launchForce = force * body->getMass();
				Physics::applyForceToPhysicsId(heldPhysicsId, launchDir * launchForce);
				std::cout << "Threw object ID: " << heldPhysicsId << "\n";
				isHold = false;
				heldPhysicsId = 0;
			}

		}
		leftMouseWasPressed = leftDown;

		if (isHold && heldPhysicsId != 0) {
			btRigidBody* body = Physics::GetBodyById(heldPhysicsId);
			if (!body) {
				isHold = false;
				heldPhysicsId = 0;
			}
			else {
				glm::vec3 holdPos = Renderer::camera.getPosition() + Renderer::camera.getFront() * holdDistance;
				Physics::forceToward(heldPhysicsId, holdPos, body->getMass() * 100.0f);
			}
		}
	}
	
}