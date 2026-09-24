#pragma once
#include "Dependencies.h"

namespace Cube
{
	inline void printVector(std::string name, std::vector<int> v)
	{
		for (auto i : v)
			std::cout << i << " ";
	}

	inline void printVec3(std::string name, glm::vec3 vec)
	{
		std::cout << "[ " << name << " Vector3:]" << " X: " << vec.x << " Y: " << vec.y << " Z: " << vec.z << std::endl;
	}
}