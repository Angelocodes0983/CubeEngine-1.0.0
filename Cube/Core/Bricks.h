#pragma once
#include "Dependencies.h"
#include "Renderer.h"


namespace Cube::Bricks
{

	inline static int BrickCount = 0;
	int getMaterial(glm::vec3 pos, glm::vec3 worldSize, const std::vector<unsigned char>& voxels);
	int brickLogic(std::vector<unsigned char>& voxels, glm::ivec3 objectSize, glm::ivec3 startPos, int brickSize);
	void constructBrickMap(std::vector<unsigned char>& voxels, glm::ivec3 objectSize, uint32_t objId);
	void updateBricks(std::vector<unsigned char>& voxels, glm::ivec3 objectSize, glm::ivec3 pos, uint32_t objId);
}