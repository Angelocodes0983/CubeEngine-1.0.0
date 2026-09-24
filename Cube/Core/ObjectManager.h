#pragma once
#include "Dependencies.h"
#include "Renderer.h"
#include "Physics.h"
#include "Meshing.h"
#include "VoxLoader.h"
#include "Bricks.h"
#include "Utility.h"
namespace Cube::ObjectManager
{
	inline uint32_t nextPhysicsId = 0;

	inline std::unordered_map<uint32_t, size_t> physicsIdToVoxelIndex;
	inline std::unordered_map<uint32_t, Material> physicsIdToMaterial;
	inline std::unordered_map<uint32_t, std::shared_ptr< std::vector<unsigned char>>> physicsIdToVoxelData;
	inline std::unordered_map<uint32_t, glm::ivec3> physicsIdToGridSize;
	inline std::unordered_map<uint32_t, glm::ivec3> physicsIdToVoxelCenter;
	inline std::unordered_map<uint32_t, Material> physicsIdToPhysicsMaterial;
	inline std::unordered_map<uint32_t, int> physicsIdToOcVoxelCount;
	inline std::unordered_map<uint32_t, glm::vec3> physicsIdToScale;
	inline std::unordered_map<uint32_t, glm::vec3> physicsIdToCentroid;

	struct BoudningBox
	{
		glm::vec3 startPos;
		glm::vec3 endPos;
		glm::vec3 center;
		uint32_t voxelCount;
	};

	float getMass(size_t id, float massPerVoxel);

	BoudningBox calculateVoxelCenter(std::vector<unsigned char>& voxels, glm::vec3 gridSize, int id, bool visable);

	void addVoxelObject(
		std::shared_ptr<std::vector<unsigned char>>voxels,
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::ivec3 gridSize,
		bool isDynamic = false,
		int hasCollider = 0,
		Material mat = Materials::Wood);

	void addVoxelObjectFromFile(
		const std::string& voxFilePath,
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		bool isDynamic = false,
		glm::ivec3 maxSize = glm::ivec3(256),
		int hasCollide = 0,
		Material mat = Materials::Wood);

	void removeObject(size_t id);
	inline static bool used = false;
	int nextPowerOfTwo(int n);

	void syncPhysicsToGPU();
}