#pragma once
#include "Dependencies.h"
#include "Renderer.h"
#include "Camera.h"

namespace Cube::Window
{
	inline bool mouseCaptured = false;
	inline uint32_t WIDTH, HEIGHT;
	struct WindowProps
	{
		uint32_t Width, Height;
		bool FullScreen;
		const char* label;

	};
	WindowProps selectResolution();
	void initWindow(const WindowProps&);

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

}