#pragma once
#include "Dependencies.h"

namespace Cube
{
	inline float deltaTime = 0;
	inline float lastTime = glfwGetTime();
	inline void timeTracker()
	{
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

	}

}