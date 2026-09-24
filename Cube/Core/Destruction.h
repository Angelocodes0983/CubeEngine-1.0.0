#pragma once
#include "Cube.h"

namespace Cube
{

	inline static int DESTRUCTIONMATERIAL = 0;
	inline static int DESTRUCTIONRADIUS =5;
	void destroyVoxelsInObjectsAtWorldPos(glm::vec3 worldCenter, float radius) {
		for (auto& [physId, voxIdx] : ObjectManager::physicsIdToVoxelIndex) {

			Renderer::VoxelObject& obj = Renderer::voxelObjects[voxIdx];
			glm::ivec3 gridSize = ObjectManager::physicsIdToGridSize[physId];
			auto& voxels = *ObjectManager::physicsIdToVoxelData.at(physId);
			glm::vec3 objPosition = ObjectManager::physicsIdToCentroid[physId];
			glm::vec3 localCenter = glm::vec3(obj.invTransform * glm::vec4(worldCenter, 1.0f));

			float dis = distance(localCenter, objPosition);

			if (dis > gridSize.x) continue;
			int iRadius = static_cast<int>(std::ceil(radius));
			float radiusSq = radius * radius;
			bool anyRemoved = false;

			auto start = std::chrono::steady_clock::now();




			for (int dz = -iRadius; dz <= iRadius; dz++) {
				for (int dy = -iRadius; dy <= iRadius; dy++) {
					for (int dx = -iRadius; dx <= iRadius; dx++) {
						if (dx * dx + dy * dy + dz * dz > radiusSq) continue;

						int x = (int)std::round(localCenter.x) + dx;
						int y = (int)std::round(localCenter.y) + dy;
						int z = (int)std::round(localCenter.z) + dz;

						if (x < 0 || y < 0 || z < 0 ||
							x >= gridSize.x || y >= gridSize.y || z >= gridSize.z)
							continue;

						size_t idx = (size_t)z * gridSize.x * gridSize.y
							+ (size_t)y * gridSize.x + x;

						if (voxels[idx] != DESTRUCTIONMATERIAL) {
							voxels[idx] = DESTRUCTIONMATERIAL;
							anyRemoved = true;
						}
					}
				}
			}


			auto end = std::chrono::steady_clock::now();

			auto time_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

			std::cout << "------------------------DESTRUCTION DEBUG------------------------" << std::endl;
			std::cout << "it takes " << time_us.count() << "micro-seconds to check the radius" << std::endl;

			auto start2 = std::chrono::steady_clock::now();

			if (anyRemoved) {

				std::cout << "Voxels removed from object " << physId << "\n";
				Renderer::createVoxelTexture(voxIdx, voxels, gridSize.x, gridSize.y, gridSize.z);
				Renderer::updateVoxelObjectDescriptor();

				//rebuildCollider(physId, voxels, gridSize);

			}

			printf("iRadius : %d \n", iRadius);
			printf("localCenter : %f, %f, %f \n", localCenter.x, localCenter.y, localCenter.z);

			Bricks::updateBricks(voxels, gridSize, glm::vec3(0), voxIdx);
			Renderer::updateBrickBuffer();

			auto end2 = std::chrono::steady_clock::now();

			auto time_us2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
			std::cout << "voxIdx " << voxIdx << std::endl;

			std::cout << "it takes " << time_us2.count() << "micro-seconds to create and upload the texture" << std::endl;
		}
	}
	inline void fracturedObject(size_t id, std::vector<unsigned char>& tempVoxels, glm::ivec3 size)
	{
		printf("this body isnt connected \n");
		btRigidBody* body;
		Physics::BodyInfo info;
		glm::vec3 position;
		glm::quat rotation;


		info = Physics::GetBodyInfoById(id);
		body = Physics::GetBodyById(id);

		if (!info.found) return;

		glm::ivec3 gridSize(
			ObjectManager::nextPowerOfTwo(size.x),
			ObjectManager::nextPowerOfTwo(size.y),
			ObjectManager::nextPowerOfTwo(size.z)
		);
		position = glm::vec3(info.position.x, info.position.y+100, info.position.z);
		rotation = info.quatRotation;
		auto voxelData = std::make_shared<std::vector<unsigned char>>(std::move(tempVoxels));
		printVec3("fracturedObject position", position);
		printVec3("fracturedObject size", gridSize);
		ObjectManager::addVoxelObject(
			voxelData,
			position,
			rotation,
			glm::vec3(1.0f),
			glm::vec3(size),
			false,
			0
		);
		
		
	}
	inline glm::ivec3 findSupportPos(std::vector<unsigned char>& voxels, glm::ivec3 size, int newColor)
	{

		for (int z = 0;z < size.z; z += 1) {
			for (int y = 0; y < size.y; y += 1) {
				for (int x = 0; x < size.x; x += 1) {

					glm::ivec3 pos = glm::ivec3(x, y, z);
					size_t index = (size_t)pos.z * (size_t)size.x * (size_t)size.y
						+ (size_t)pos.y * (size_t)size.x
						+ (size_t)pos.x;
					if (voxels[index] == newColor || voxels[index] == 0) { continue; }
					return pos;
				}
			}
			//std::cout << "bricks: " << bricks << std::endl;
		}
	}
	inline void floodFill(std::vector<unsigned char>& voxels, int newColor, glm::ivec3 size, size_t id) {

		uint32_t vistedVoxels = 0;
		glm::ivec3 validPos = findSupportPos(voxels, size, newColor);
		std::vector<unsigned char> tempVoxels(size.x * size.y * size.z);

		bool connected = false;
		// If the starting pixel already has the new color
		size_t index = (size_t)validPos.z * (size_t)size.x * (size_t)size.y
			+ (size_t)validPos.y * (size_t)size.x
			+ (size_t)validPos.x;


		int minX = size.x;
		int minY = size.y;
		int minZ = size.z;
		int maxX = 0;
		int maxY = 0;
		int maxZ = 0;

		if (voxels[index] == newColor || voxels[index] == 0)
		{
			return;
		}

		//std::cout << "floodFill running \n";
		// Direction vectors for traversing 6 directions
		std::vector< glm::vec3> dir = { glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1) };

		std::queue <glm::vec3> q;

		//std::cout << "START POSITION" << validPos.x << " x " << validPos.y << " y " << validPos.z << " z " << std::endl;

		//printf("oldColor %d", oldColor);
		q.push(validPos);

		// Change the color of the starting pixel
		//voxels[index] = newColor;

		// Perform BFS
		while (!q.empty()) {
			glm::vec3 front = q.front();
			int x = front.x, y = front.y, z = front.z;
			q.pop();

			// Traverse all 6 directions
			for (glm::vec3& it : dir) {

				//std::cout << x << " x " << y << " y " << z << " z " << std::endl;
				int nx = x + it.x;
				int ny = y + it.y;
				int nz = z + it.z;

				//std::cout << it.x << " Itx " << std::endl;

				//std::cout << "loop \n";
				//std::cout << nx << " x " << ny << " y " << nz << " z " << std::endl;


				size_t localIndex = (size_t)nz * (size_t)size.x * (size_t)size.y
					+ (size_t)ny * (size_t)size.x
					+ (size_t)nx;

				// Check boundary conditions and color match
				if (nx >= 0 && nx < size.x &&
					ny >= 0 && ny < size.y &&
					nz >= 0 && nz < size.z &&
					voxels[localIndex] != newColor && voxels[localIndex] != 0) {

					//std::cout << "converted \n";
					tempVoxels[localIndex] = voxels[localIndex];
					voxels[localIndex] = newColor;
					vistedVoxels++;
					q.push(glm::vec3(nx, ny, nz));

					minX = glm::min(nx, minX);
					minY = glm::min(ny, minY);
					minZ = glm::min(nz, minZ);
					maxX = glm::max(nx, maxX);
					maxY = glm::max(ny, maxY);
					maxZ = glm::max(nz, maxZ);
				}
			}


		}



		if (vistedVoxels >= voxels.size())
		{
			//printf("this body is whole \n");

		}
		else
		{
			//printf("this body isnt whole \n");

			fracturedObject(id, tempVoxels, glm::ivec3(maxX, maxY, maxZ));
			//floodFill(voxels, newColor, size, id);
		}


	}

	inline void destructionSphere(size_t id, glm::vec3 pos, float radius)
	{

		Renderer::VoxelObject& obj = Renderer::voxelObjects[id];
		glm::ivec3 gridSize = ObjectManager::physicsIdToGridSize[id];
		auto& voxels = *ObjectManager::physicsIdToVoxelData.at(id);
		glm::vec3 objPosition = ObjectManager::physicsIdToCentroid[id];
		glm::vec3 localCenter = glm::vec3(obj.invTransform * glm::vec4(pos, 1.0f));

		float dis = distance(localCenter, objPosition);

		if (dis > gridSize.x) return;
		int iRadius = static_cast<int>(std::ceil(radius));
		float radiusSq = radius * radius;
		bool anyRemoved = false;

		auto start = std::chrono::steady_clock::now();




		for (int dz = -iRadius; dz <= iRadius; dz++) {
			for (int dy = -iRadius; dy <= iRadius; dy++) {
				for (int dx = -iRadius; dx <= iRadius; dx++) {
					if (dx * dx + dy * dy + dz * dz > radiusSq) continue;

					int x = (int)std::round(localCenter.x) + dx;
					int y = (int)std::round(localCenter.y) + dy;
					int z = (int)std::round(localCenter.z) + dz;

					if (x < 0 || y < 0 || z < 0 ||
						x >= gridSize.x || y >= gridSize.y || z >= gridSize.z)
						continue;

					size_t idx = (size_t)z * gridSize.x * gridSize.y
						+ (size_t)y * gridSize.x + x;

					if (voxels[idx] != DESTRUCTIONMATERIAL) {
						voxels[idx] = DESTRUCTIONMATERIAL;
						anyRemoved = true;
					}
				}
			}
		}


		if (anyRemoved) {

			std::cout << "Voxels removed from object " << id << "\n";
			Renderer::createVoxelTexture(id, voxels, gridSize.x, gridSize.y, gridSize.z);
			Renderer::updateVoxelObjectDescriptor();

			//rebuildCollider(physId, voxels, gridSize);

		}

		printf("iRadius : %d \n", iRadius);
		printf("localCenter : %f, %f, %f \n", localCenter.x, localCenter.y, localCenter.z);

		Bricks::updateBricks(voxels, gridSize, glm::vec3(0), id);
		Renderer::updateBrickBuffer();
	}

}