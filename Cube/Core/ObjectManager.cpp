#include "ObjectManager.h"


namespace Cube::ObjectManager
{


	void addVoxelObject(
		std::shared_ptr<std::vector<unsigned char>>voxels,
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::ivec3 gridSize,
		bool isDynamic,
		int hasCollider,
		Material mat)
	{

		if (Cube::Renderer::voxelObjects.size() >= Cube::Renderer::MAX_VOXEL_OBJECTS) { return; }


		uint32_t texIndex = static_cast<uint32_t>(Cube::Renderer::voxelObjects.size());
		Cube::Renderer::createVoxelTexture(texIndex, *voxels, gridSize.x, gridSize.y, gridSize.z);
		glm::vec3 center = glm::vec3(gridSize) * 0.5f;
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 R = glm::mat4_cast(rotation);
		glm::mat4 T = glm::translate(glm::mat4(1.0f), position + center);
		glm::mat4 TBack = glm::translate(glm::mat4(1.0f), -center);
		glm::vec3 gridOrigin = position;
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), gridOrigin) * R * S;
		glm::mat4 inv = glm::inverse(transform);

		Cube::Renderer::VoxelObject obj{};
		obj.transform = transform;
		obj.invTransform = inv;
		obj.mass = 1;
		obj.restitution = 2.0f;
		obj.gridSizeAndTex = glm::ivec4(gridSize, static_cast<int>(texIndex));
		obj.boundsMin = glm::vec3(0.0f);
		obj.boundsMax = glm::vec3(gridSize);
		obj.velocity = glm::vec3(0.0f);
		obj.pad0 = 0.0f;
		obj.angularVel = glm::vec3(0.0f);
		obj.pad1 = 0.0f;

		Meshing::Mesh mesh;

		if (hasCollider == 0)
		{
			std::cout << "Generating mesh for Id" << texIndex << "\n";
			mesh = Meshing::greedyMesh(gridSize, *voxels);

		}
		else if (hasCollider == 1)
		{
			mesh = Meshing::meshBox(glm::vec3(0), gridSize);
		}
		else if (hasCollider == 2)
		{
	
			mesh = Meshing::planeMesh(0);
		}

		//for (auto vert : mesh.vertices)
		//{
			//std::cout << "[ Vert Position Vector3:]" << " X: " << vert.x << " Y: " << vert.y << " Z: " << vert.z << std::endl;

		//}

		Cube::Renderer::voxelObjects.push_back(obj);

		Cube::Bricks::constructBrickMap(*voxels, gridSize, texIndex);

		std::cout << "creating object with voxelSize:" << (*voxels).size() << std::endl;


		if (!mesh.vertices.empty()) {
			std::cout << "has mesh\n";


			std::vector<glm::vec3> positions;
			positions.reserve(mesh.vertices.size());
			std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(positions),
				[](const glm::vec3 v) { return v; });


			glm::vec3 localCentroid(0.0f);
			for (const auto& p : positions) localCentroid += p;
			localCentroid /= (float)positions.size();


			for (auto& p : positions) p -= localCentroid;

			 
			glm::vec3 scaledCentroid = center * scale;
			glm::vec3 rotatedCentroid = glm::vec3(R * glm::vec4(scaledCentroid, 0.0f));

			glm::vec3 physicsPos = position + rotatedCentroid;
			glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(rotation));



			uint32_t physId = nextPhysicsId++;



			//std::string file = globalFile;
			//if (physId == 0 && saving == true) {
				//saveVoxels(file, gridSize, *voxels);
			//}


			std::cout << "before emplace size " << (*voxels).size() << std::endl;;
			physicsIdToVoxelIndex[physId] = texIndex;
			physicsIdToVoxelData.emplace(physId, voxels);
			std::cout << "after emplace size " << (*voxels).size() << std::endl;;

			physicsIdToGridSize[physId] = gridSize;
			physicsIdToScale[physId] = scale;
			physicsIdToCentroid[physId] = localCentroid;
			glm::vec3 voxCenter = calculateVoxelCenter(*physicsIdToVoxelData.at(physId), physicsIdToGridSize[physId], physId, false).center;
			physicsIdToVoxelCenter[physId] = voxCenter;
			physicsIdToPhysicsMaterial[physId] = mat;

			::Cube::Material material;
			material.mass = isDynamic ? getMass(physId, mat.mass / 100) : 0.0f;
			material.friction = mat.friction;

			std::cout << "creating customshape" << std::endl;

			std::cout << "positions size" << positions.size() << std::endl;

			Cube::Physics::AddCustomShapeFromMesh(
				physId,
				physicsPos,
				eulerDeg,
				material,
				isDynamic,
				glm::bvec3(false),
				true,
				positions,
				scale
			);

			//btRigidBody* body = Cube::Physics::GetBodyById(physId);
			//body->setActivationState(DISABLE_SIMULATION);

		}
	}

	int nextPowerOfTwo(int n) {
		n--;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		return n + 1;
	}
	void addVoxelObjectFromFile(
		const std::string& voxFilePath,
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		bool isDynamic,
		glm::ivec3 maxSize,
		int hasCollide,
		Material mat)
	{
		if (Cube::Renderer::voxelObjects.size() >= Cube::Renderer::MAX_VOXEL_OBJECTS) { return; }
		auto voxModels = VoxLoader::loadAll(voxFilePath, 2000);
		if (voxModels.empty())
			throw std::runtime_error("No models found in .vox file: " + voxFilePath);

		glm::ivec3 globalMin(INT_MAX);
		for (const auto& m : voxModels)
			globalMin = glm::min(globalMin, m.worldOffset);

		for (auto& m : voxModels)
			m.worldOffset -= globalMin;

		glm::ivec3 sceneMax(0);
		for (const auto& m : voxModels) {
			glm::ivec3 offset(m.worldOffset.x, m.worldOffset.z, m.worldOffset.y);
			sceneMax = glm::max(sceneMax, offset + glm::ivec3(m.sizeX, m.sizeZ, m.sizeY));
		}

		glm::ivec3 exactSize(
			std::max(sceneMax.x, 1),
			std::max(sceneMax.y, 1),
			std::max(sceneMax.z, 1)
		);


		glm::ivec3 gridSize(
			nextPowerOfTwo(exactSize.x),
			nextPowerOfTwo(exactSize.y),
			nextPowerOfTwo(exactSize.z)
		);





		std::cout << "[addVoxelObjectFromFile] File: " << voxFilePath << "\n";
		std::cout << "[addVoxelObjectFromFile] Scene bounds: "
			<< sceneMax.x << "x" << sceneMax.y << "x" << sceneMax.z << "\n";
		std::cout << "[addVoxelObjectFromFile] Grid size: "
			<< gridSize.x << "x" << gridSize.y << "x" << gridSize.z << "\n";


		auto voxels = std::make_shared<std::vector<unsigned char>>((size_t)gridSize.x * gridSize.y * gridSize.z, 0);

		for (const auto& m : voxModels) {
			glm::ivec3 offset(m.worldOffset.x, m.worldOffset.z, m.worldOffset.y);

			auto modelVoxels = VoxLoader::toVoxelArray(
				m,
				gridSize.x, gridSize.y, gridSize.z,
				offset,
				true
			);


			for (size_t j = 0; j < voxels->size(); j++) {
				if (modelVoxels[j] != 0)
					(*voxels)[j] = modelVoxels[j];
			}
		}

		int filledCount = 0;
		for (auto v : (*voxels)) if (v != 0) filledCount++;
		std::cout << "[addVoxelObjectFromFile] Filled voxels: " << filledCount
			<< " / " << voxels->size()
			<< " (" << (100.0f * filledCount / voxels->size()) << "%)\n";


		addVoxelObject(voxels, position, rotation, scale, gridSize, isDynamic, hasCollide, mat);
	}

	BoudningBox calculateVoxelCenter(std::vector<unsigned char>& voxels, glm::vec3 gridSize, int id, bool visable)
	{
		std::cout << "calculateVoxelCenter voxelSize" << voxels.size() << std::endl;

		float minX = gridSize.x;
		float minY = gridSize.y;
		float minZ = gridSize.z;
		float maxX = 0;
		float maxY = 0;
		float maxZ = 0;

		int solidCount = 0;
		for (int z = 0; z < gridSize.z; z++) {
			for (int y = 0; y < gridSize.y; y++) {
				for (int x = 0; x < gridSize.x; x++) {

					glm::vec3 pos = glm::vec3(x, y, z);
					if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
						pos.x >= gridSize.x || pos.y >= gridSize.y || pos.z >= gridSize.z)
						continue;
					size_t idx = (size_t)pos.z * gridSize.x * gridSize.y
						+ (size_t)pos.y * gridSize.x + pos.x;
					if (voxels[idx] == 0)
						continue;
					minX = glm::min(pos.x, minX);
					minY = glm::min(pos.y, minY);
					minZ = glm::min(pos.z, minZ);
					maxX = glm::max(pos.x, maxX);
					maxY = glm::max(pos.y, maxY);
					maxZ = glm::max(pos.z, maxZ);

					solidCount++;

				}
			}
		}
		physicsIdToOcVoxelCount[id] = solidCount;
		glm::vec3 center = glm::vec3((minX + maxX) / 2, (minX + maxX) / 2, (minX + maxX) / 2);

		if (visable)
		{
			for (int z = 0; z < gridSize.z; z++) {
				for (int y = 0; y < gridSize.y; y++) {
					for (int x = 0; x < gridSize.x; x++) {

						glm::vec3 pos = glm::vec3(x, y, z);
						if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
							pos.x >= gridSize.x || pos.y >= gridSize.y || pos.z >= gridSize.z)
							continue;
						size_t idx = (size_t)pos.z * gridSize.x * gridSize.y
							+ (size_t)pos.y * gridSize.x + pos.x;

						if (pos.x < minX || pos.y < minY || pos.z < minZ ||
							pos.x >= maxX + 1 || pos.y >= maxY + 1 || pos.z >= maxZ + 1)
							continue;


						if (pos.x == minX && pos.z == minZ || pos.x == maxX && pos.z == minZ ||
							pos.x == maxX && pos.z == maxZ || pos.x == minX && pos.z == maxZ ||
							pos.y == maxY && pos.z == minZ || pos.y == maxY && pos.z == maxZ ||
							pos.y == maxY && pos.x == maxX || pos.y == maxY && pos.x == minX ||
							pos.y == minY && pos.z == minZ || pos.y == minY && pos.z == maxZ ||
							pos.y == minY && pos.x == minX || pos.y == minY && pos.x == maxX)
						{
							voxels[idx] = 176;
						}

					}
				}
			}

			//createVoxelTexture(id, voxels, gridSize.x, gridSize.y, gridSize.z);

			std::cout << "created tex" << std::endl;
			//updateVoxelObjectDescriptor();
			std::cout << "updated descriptor" << std::endl;

			//rebuildCollider(physId, voxels, gridSize);



			//updateBricks(voxels, gridSize, glm::vec3(0), id);
			std::cout << "updated bricks" << std::endl;

			//updateBrickBuffer();
		}



		BoudningBox area;
		area.startPos = glm::vec3(minX, minY, minZ);
		area.endPos = glm::vec3(maxX, maxY, maxZ);
		area.center = center;
		return area;
		std::cout << "minx" << minX << std::endl;
	}
	float getMass(size_t id, float massPerVoxel)
	{
		int voxelCount = physicsIdToOcVoxelCount[id];

		return voxelCount * massPerVoxel;

		std::cout << " id" << id << " has mass" << voxelCount * massPerVoxel << std::endl;
	}


	void syncPhysicsToGPU() {
		if (!Cube::Physics::dynamicsWorld) return;
		bool anyChanged = false;
		namespace R = Cube::Renderer;
		for (auto& [physId, voxIdx] : physicsIdToVoxelIndex) {

			
			btRigidBody* body = Cube::Physics::GetBodyById(physId);

			if (!body) continue;

			btTransform btTrans;
			body->getMotionState()->getWorldTransform(btTrans);

			btVector3 btPos = btTrans.getOrigin();
			btQuaternion btRot = btTrans.getRotation();

			glm::vec3 worldPos(btPos.x(), btPos.y(), btPos.z());
			glm::quat rot(btRot.w(), btRot.x(), btRot.y(), btRot.z());
			glm::mat4 R = glm::mat4_cast(rot);

			glm::vec3 scale(1.0f);
			auto scIt = physicsIdToScale.find(physId);
			if (scIt != physicsIdToScale.end()) scale = scIt->second;

			glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

			R::VoxelObject& obj = R::voxelObjects[voxIdx];

			glm::vec3 localCentroid(0.0f);

			auto centIt = physicsIdToCentroid.find(physId);
			if (centIt != physicsIdToCentroid.end())
				localCentroid = centIt->second;

			glm::vec3 scaledCentroid = localCentroid * scale;
			glm::vec3 rotatedCentroid = glm::vec3(R * glm::vec4(scaledCentroid, 0.0f));
			glm::vec3 gridOrigin = worldPos - rotatedCentroid;

			obj.transform = glm::translate(glm::mat4(1.0f), gridOrigin) * R * S;
			obj.invTransform = glm::inverse(obj.transform);
			anyChanged = true;
		}

		if (anyChanged) {
			vkWaitForFences(VkDevice(R::device), 1, &R::graphicsFence, VK_TRUE, UINT64_MAX);
			vkWaitForFences(VkDevice(R::device), 1, &R::computeFence, VK_TRUE, UINT64_MAX);

			VkDeviceSize size = sizeof(R::VoxelObject) * R::voxelObjects.size();

			void* data;
			vkMapMemory(R::device, R::voxelObjectBufferMemory, 0, size, 0, &data);
			memcpy(data, R::voxelObjects.data(), size);
			vkUnmapMemory(R::device, R::voxelObjectBufferMemory);

		}

	}

	void removeObject(size_t id)
	{

		// Check if the target index was found
		if (!used)
		{
			std::cout << "remove ran" << std::endl;
			Physics::RemoveBodyById(id);
			Cube::Renderer::voxelObjects.erase(Cube::Renderer::voxelObjects.begin() + id);

			used = true;
		}

	}
}