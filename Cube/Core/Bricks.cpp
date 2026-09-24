#include "Bricks.h"

namespace Cube::Bricks
{


	int getMaterial(glm::vec3 pos, glm::vec3 worldSize, const std::vector<unsigned char>& voxels) {
		if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
			pos.x >= worldSize.x ||
			pos.y >= worldSize.y ||
			pos.z >= worldSize.z) {


			return false;
		}

		size_t index = (size_t)pos.z * (size_t)worldSize.x * (size_t)worldSize.y
			+ (size_t)pos.y * (size_t)worldSize.x
			+ (size_t)pos.x;




		if (index >= voxels.size())
		{
			return false;
		}

		return voxels[index];
	}


	int brickLogic(std::vector<unsigned char>& voxels, glm::ivec3 objectSize, glm::ivec3 startPos, int brickSize) {

		int material;
		int startMaterial = getMaterial(glm::vec3(startPos), glm::vec3(objectSize), voxels);

		glm::vec3 pos = glm::vec3(startPos);

		for (int bz = pos.z;bz < pos.z + brickSize; bz++) {
			for (int by = pos.y;by < pos.y + brickSize; by++) {
				for (int bx = pos.x;bx < pos.x + brickSize; bx++) {



					material = getMaterial(glm::vec3(bx, by, bz), glm::vec3(objectSize), voxels);


					if (startMaterial != material)
					{
						return 500;
					}


				}
			}
		}

		return startMaterial;


	}



	void constructBrickMap(std::vector<unsigned char>& voxels, glm::ivec3 objectSize, uint32_t objId) {

		int full = 0;
		int empty = 0;
		int mixed = 0;
		int brickSize = 8;
		int bricks = 0;

		Cube::Renderer::BrickNode defualtNode;

		defualtNode.occupancy = 0;

		std::vector<Cube::Renderer::BrickNode> defualNodeList = { defualtNode };
		Cube::Renderer::BrickNodes.resize(Cube::Renderer::voxelObjects.size(), defualNodeList);
		Cube::Renderer::brickElements.resize(Cube::Renderer::voxelObjects.size(), 0);
		Cube::Renderer::BrickNodes[objId].resize((objectSize.x / 8) * (objectSize.y / 8) * (objectSize.z / 8), defualtNode);
		//loop through every brick
		for (int z = 0;z < objectSize.z; z += brickSize) {
			for (int y = 0; y < objectSize.y; y += brickSize) {
				for (int x = 0; x < objectSize.x; x += brickSize) {

					bricks++;
					BrickCount++;
					//logic for individual brick


					int mat = brickLogic(voxels, objectSize, glm::ivec3(x, y, z), brickSize);
					//brickBitMap(voxels, objectSize, glm::ivec3(x, y, z), brickSize, tempBitMap);


					int oc = 0;
					//set occupancy
					if (mat == 500)
					{
						mixed++;
					}
					else if (mat == 0)
					{
						empty++;
					}
					else if (mat > 0)
					{
						full++;
					}

					oc = mat;

					glm::ivec3 localBrickPos = glm::ivec3(x, y, z) / 8;
					glm::ivec3 localBWSize = objectSize / 8;

					if (localBrickPos.x < 0 ||
						localBrickPos.y < 0 ||
						localBrickPos.z < 0) {
						return;
					}

					size_t brickIndex =
						(size_t)localBrickPos.z * (size_t)localBWSize.x * (size_t)localBWSize.y +
						(size_t)localBrickPos.y * (size_t)localBWSize.x +
						(size_t)localBrickPos.x;



					if (brickIndex >= Cube::Renderer::BrickNodes[objId].size())
					{
						continue;
					}



					Cube::Renderer::BrickNodes[objId][brickIndex].occupancy = oc;


					if (bricks == 1)
					{
						Cube::Renderer::brickElements[objId] = BrickCount - 1;

					}



				}
			}
			std::cout << "bricks: " << bricks << std::endl;
		}
		std::cout << "full bricks: " << full << std::endl;
		std::cout << "empty bricks: " << empty << std::endl;
		std::cout << "mixed bricks: " << mixed << std::endl;

	}

	void updateBricks(std::vector<unsigned char>& voxels, glm::ivec3 objectSize, glm::ivec3 pos, uint32_t objId) {

		int full = 0;
		int empty = 0;
		int mixed = 0;
		int brickSize = 8;
		int bricks = 0;



		for (int z = pos.x;z < objectSize.z + pos.z; z += brickSize) {
			for (int y = pos.y; y < objectSize.y + pos.y; y += brickSize) {
				for (int x = pos.z; x < objectSize.x + pos.x; x += brickSize) {


					bricks++;
					BrickCount++;
					//logic for individual brick


					int mat = brickLogic(voxels, objectSize, glm::ivec3(x, y, z), brickSize);



					int oc = 0;
					//set occupancy  
					if (mat == 500)
					{
						mixed++;
					}
					else if (mat == 0)
					{
						empty++;
					}
					else if (mat > 0)
					{
						full++;
					}

					oc = mat;

					glm::ivec3 localBrickPos = glm::ivec3(x, y, z) / 8;
					glm::ivec3 localBWSize = objectSize / 8;

					if (localBrickPos.x < 0 ||
						localBrickPos.y < 0 ||
						localBrickPos.z < 0) {
						return;
					}

					size_t brickIndex =
						(size_t)localBrickPos.z * (size_t)localBWSize.x * (size_t)localBWSize.y +
						(size_t)localBrickPos.y * (size_t)localBWSize.x +
						(size_t)localBrickPos.x;



					if (brickIndex >= Renderer::BrickNodes[objId].size())
					{
						continue;
					}



					Renderer::BrickNodes[objId][brickIndex].occupancy = oc;



				}
			}
			//std::cout << "bricks: " << bricks << std::endl;
		}
		std::cout << "full bricks: " << full << std::endl;
		std::cout << "empty bricks: " << empty << std::endl;
		std::cout << "mixed bricks: " << mixed << std::endl;

		printf("Updated bricks: %d \n", bricks);



	}
}