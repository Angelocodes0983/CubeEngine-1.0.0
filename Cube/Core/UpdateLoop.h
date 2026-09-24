#pragma once
#include "Dependencies.h"
#include "ObjectManager.h"
#include "ProcessInput.h"
#include "Time.h"
#include "Ui.h"
#include "TestApp.h"
namespace Cube
{
	inline void mainLoop() {
		std::cout << "Application running with swapchain!\n";
		std::cout << "Press ESC or close window to exit\n";


		while (!glfwWindowShouldClose(Cube::Renderer::window)) {
			glfwPollEvents();


			timeTracker();

			Cube::Physics::Update(deltaTime);
			

			Cube::ObjectManager::syncPhysicsToGPU();

			processInput();

			appInput();

			if (glfwGetKey(Cube::Renderer::window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				glfwSetWindowShouldClose(Cube::Renderer::window, true);
			}
			uiFrame();
			if (enableDebugUi) { debugUi(); }
			if (enableOptionsUi) { optionsUi(); }


			ImGui::Render();
			Cube::Renderer::drawFrame();

		}


		vkDeviceWaitIdle(Cube::Renderer::device);
		std::cout << "Rendering completed\n";
	}
}