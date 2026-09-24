#pragma once
#include "Dependencies.h"
#include "KeyCodes.h"
#include "MouseCodes.h"
#include "Renderer.h"
namespace Cube
{	

	class Input
	{
	public:	
		
		static bool IsKeyPressed(const KeyCode key);

		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();


	};

}