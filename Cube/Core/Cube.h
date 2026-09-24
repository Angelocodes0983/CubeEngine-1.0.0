#pragma once
#include "Window.h"
#include "Renderer.h"
#include "UpdateLoop.h"
#include "TestApp.h"

namespace Cube
{
	inline void cube()
	{
		Cube::Window::initWindow(Cube::Window::selectResolution());
		Cube::Physics::intialize();
		Cube::Renderer::initVulkan();
		Cube::createTestSetup();

		Cube::mainLoop();
		Cube::Renderer::cleanup();

	}
}