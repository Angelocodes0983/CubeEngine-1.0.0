#pragma once
#include "Cube.h"

namespace Cube
{
	inline static bool enableOptionsUi = true;
	inline static bool enableDebugUi = true;

	inline glm::vec3 color;

	inline void uiFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}


	inline void debugUi()
	{

		ImGui::Begin("Debug");

		// --- Performance ---
		ImGui::Text("FPS: %.1f  |  Frame: %.2f ms", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Separator();

		ImGui::Separator();

		ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
			Renderer::camera.getPosition().x, Renderer::camera.getPosition().y, Renderer::camera.getPosition().z);

		// --- Camera ---
		if (ImGui::CollapsingHeader("Camera")) {
			auto pos = Renderer::camera.getPosition();
			auto front = Renderer::camera.getFront();
			ImGui::Text("Pos:   (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
			ImGui::Text("Front: (%.2f, %.2f, %.2f)", front.x, front.y, front.z);
			ImGui::Text("Zoom:  %.1f", Renderer::camera.getZoom());
			//ImGui::Text("Mouse: %s", mouseCaptured ? "Captured (O to release)" : "Free (O to capture)");
		}

		// --- Scene ---
		if (ImGui::CollapsingHeader("Scene")) {
			ImGui::Text("Voxel objects: %d", (int)Renderer::voxelObjects.size());
			ImGui::Text("Octree nodes:  %d", (int)Renderer::BrickNodes.size());
			ImGui::Text("CurrentTime: %f", deltaTime);
			for (size_t i = 0; i < Renderer::voxelObjects.size(); i++) {
				const auto& obj = Renderer::voxelObjects[i];
				glm::ivec3 gs = glm::ivec3(obj.gridSizeAndTex);
				ImGui::Text("  [%d] grid %dx%dx%d  tex=%d  mass=%.1f",
					(int)i, gs.x, gs.y, gs.z, obj.gridSizeAndTex.w, obj.mass);
			}
		}

		// --- Physics ---
		if (ImGui::CollapsingHeader("Physics")) {
			ImGui::Text("Bodies tracked: %d", (int)ObjectManager::physicsIdToVoxelIndex.size());

			for (auto& [physId, voxIdx] : ObjectManager::physicsIdToVoxelIndex) {
				Physics::BodyInfo info = Physics::GetBodyInfoById(physId);
				if (!info.found) continue;
				ImGui::Text("  ID %u  pos(%.0f,%.0f,%.0f)  %s",
					physId,
					info.position.x, info.position.y, info.position.z,
					info.isDynamic ? "dynamic" : "static");
			}
		}

		// --- Controls reminder ---
		if (ImGui::CollapsingHeader("Controls")) {
			ImGui::BulletText("WASD / Space / Shift  - move");
			ImGui::BulletText("Ctrl                  - speed up");
			ImGui::BulletText("Q                     - reset speed");
			ImGui::BulletText("Scroll                - destroy radius");
			ImGui::BulletText("LMB                   - destroy voxels");
			ImGui::BulletText("RMB                   - grab / release");
			ImGui::BulletText("LMB (held)            - throw");
			ImGui::BulletText("O                     - toggle mouse");
			ImGui::BulletText("ESC                   - quit");
		}

		ImGui::End();


	}
	inline int colorPickUiLogic(glm::vec3 col)
	{
		float leastDif = 100;
		int mostAccurrateIndex = 0;
		for (int i = 0; i < Renderer::globalPalette.size(); i++)
		{
			float dif = 0;
			float xA = Renderer::globalPalette[i].color.x - col.x;
			float yA = Renderer::globalPalette[i].color.y - col.y;
			float zA = Renderer::globalPalette[i].color.z - col.z;

			dif = (xA * xA + yA * yA + zA * zA);


			if (dif < leastDif) {
				leastDif = dif;
				mostAccurrateIndex = i;
			}
		}


		if (mostAccurrateIndex == 0)
			return 255;
		else
			return mostAccurrateIndex;
	}
	inline void optionsUi()
	{

		ImGuiStyle& style = ImGui::GetStyle();

		//style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
		//style.Colors[ImGuiCol_Button] = ImVec4(10.0f / 256, 10.0f / 256, 190.0f / 256, 1.0f);
		//style.Colors[ImGuiCol_ButtonHovered] = ImVec4(20.0f / 256, 20.0f / 256, 200.0f / 256, 1.0f);
		//style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		//style.ChildBorderSize = 0.0f;
		//style.FrameBorderSize = 2.0f;
		//style.FrameRounding = 8.0f;
		//style.ChildRounding = 8.0f;
		//style.WindowRounding = 8.0f;
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		//std::cout << "screensizex" << screenSize.x << "screeny" << screenSize.y << std::endl;
		ImVec2 windowSize = ImVec2(screenSize.x / 4.5, screenSize.y / 1.8);
		ImVec2 windowPos = ImVec2((screenSize.x / 2 - windowSize.x / 2) - (screenSize.x * 0.37), (screenSize.y / 2 - windowSize.y / 2));
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

		ImGui::Begin("Options", &enableOptionsUi, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

		ImGui::SliderInt("radius", &DESTRUCTIONRADIUS, 0, 100);
		//if (ImGui::ColorPicker3("picker", &color.x)) { DESTRUCTIONMATERIAL = colorPickUiLogic(color); }
		ImGui::SliderInt("material", &DESTRUCTIONMATERIAL, 0, 255);

		
		ImGui::End();

	}

}