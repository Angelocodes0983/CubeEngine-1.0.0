#include "Physics.h"

namespace Cube::Physics
{


	void intialize()
	{
		broadphase = new btDbvtBroadphase();
		collisionConfig = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfig);
		solver = new btSequentialImpulseConstraintSolver();
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
		dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
	}
	glm::vec3 bulletToGlm(const btVector3& v) { return glm::vec3(v.getX(), v.getY(), v.getZ()); }
	btVector3 glmToBullet(const glm::vec3& v) { return btVector3(v.x, v.y, v.z); }

	float distance(glm::vec3& position1, glm::vec3& position2)
	{

		float distance = sqrt((pow((position2.x - position1.x), 2) + pow((position2.y - position1.y), 2) + pow((position2.z - position1.z), 2)));
		return distance;
	}

	size_t positiontoobject(glm::vec3& position)
	{
		for (auto& [id, body] : customShapes) {
			glm::vec3 vec3bodyposition;


			const btVector3 bodyposition = body->getCenterOfMassPosition();
			vec3bodyposition = bulletToGlm(bodyposition);

			if (distance(vec3bodyposition, position) < 5)
			{
				return id;
			}




		}
	}
	void AddCube(size_t cubeId,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const Material& material,
		bool isDynamic,
		const glm::bvec3& lockRotation,
		bool render)
	{
		if (isDynamic) {
			dynamicCubeTransforms.emplace_back(position, rotation);
		}
		else {
			staticCubeTransforms.emplace_back(position, rotation);
		}

		materials.push_back(material);
		renderFlags.push_back(render);

		btCollisionShape* cubeShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

		btTransform cubeTransform;
		cubeTransform.setIdentity();
		cubeTransform.setOrigin(btVector3(position.x, position.y, position.z));

		btQuaternion rot;
		rot.setEulerZYX(glm::radians(rotation.z), glm::radians(rotation.y), glm::radians(rotation.x));
		cubeTransform.setRotation(rot);

		btScalar mass = isDynamic ? material.mass : 0.0f;
		btVector3 localInertia(0, 0, 0);
		if (mass != 0.0f) {
			cubeShape->calculateLocalInertia(mass, localInertia);
		}

		btDefaultMotionState* cubeMotionState = new btDefaultMotionState(cubeTransform);
		btRigidBody::btRigidBodyConstructionInfo cubeRbInfo(mass, cubeMotionState, cubeShape, localInertia);
		btRigidBody* cubeBody = new btRigidBody(cubeRbInfo);

		if (isDynamic) {
			cubeBody->setAngularFactor(btVector3(
				lockRotation.x ? 0.0f : 1.0f,
				lockRotation.y ? 0.0f : 1.0f,
				lockRotation.z ? 0.0f : 1.0f
			));
		}

		cubeBody->setFriction(material.friction);

		if (!isDynamic) {
			cubeBody->setCollisionFlags(cubeBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
		}

		dynamicsWorld->addRigidBody(cubeBody);
		physicsBodies.push_back(cubeBody);


		idToBodies[cubeId].push_back(cubeBody);
		bodyToId[cubeBody] = cubeId;
		bodyIsDynamic[cubeBody] = isDynamic;
	}



	void RemoveCubeById(size_t cubeId, btRigidBody* playerBody) {
		auto it = idToBodies.find(cubeId);
		if (it == idToBodies.end()) return;

		for (btRigidBody* body : it->second) {
			if (body == playerBody) continue;


			dynamicsWorld->removeRigidBody(body);
			delete body->getMotionState();
			delete body->getCollisionShape();
			delete body;


			auto bodyIt = std::find(physicsBodies.begin(), physicsBodies.end(), body);
			if (bodyIt != physicsBodies.end()) {
				size_t bodyIndex = size_t(bodyIt - physicsBodies.begin());

				physicsBodies[bodyIndex] = nullptr;
				if (bodyIndex < renderFlags.size()) {
					renderFlags[bodyIndex] = false;
				}

			}

			bodyToId.erase(body);
			bodyIsDynamic.erase(body);
		}

		idToBodies.erase(it);
	}

	void RemoveAllBodies() {

		std::vector<size_t> customShapeIdsToRemove;
		for (auto& [id, body] : customShapes) {
			customShapeIdsToRemove.push_back(id);
		}
		for (size_t id : customShapeIdsToRemove) {
			RemoveCustomShape(id);
		}


		for (btRigidBody* body : physicsBodies) {
			dynamicsWorld->removeRigidBody(body);
			delete body->getMotionState();
			delete body->getCollisionShape();
		}
		physicsBodies.clear();
		staticCubeTransforms.clear();
		dynamicCubeTransforms.clear();
		materials.clear();
		renderFlags.clear();
		bodyToId.clear();
		idToBodies.clear();
	}
	struct DebugContactCallback : public btCollisionWorld::ContactResultCallback {
		btDiscreteDynamicsWorld* world;
		float deltaTime;

		DebugContactCallback(btDiscreteDynamicsWorld* w, float dt)
			: world(w), deltaTime(dt) {
		}

		btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
			const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
		{
			const btCollisionObject* objA = colObj0Wrap->getCollisionObject();
			const btCollisionObject* objB = colObj1Wrap->getCollisionObject();

			const btVector3& ptA = cp.getPositionWorldOnA();
			const btVector3& ptB = cp.getPositionWorldOnB();


			btScalar impulse = cp.getAppliedImpulse();

			btScalar power = (deltaTime > 0.0f) ? impulse / deltaTime : impulse;


			return 0;
		}
	};
	float mapVolume(float power, float minPower, float maxPower) {
		float vol = (power - minPower) / (maxPower - minPower);
		if (vol > 1.0f) vol = 1.0f;
		if (vol < 0.0f) vol = 0.0f;
		return vol;
	}


	float mapPitch(float power, float minPower, float maxPower) {
		float pitch = 0.5f + ((power - minPower) / (maxPower - minPower)) * 1.5f;
		if (pitch > 2.0f) pitch = 2.0f;
		if (pitch < 0.5f) pitch = 0.5f;
		return pitch;
	}
	void Update(float deltaTime) {


		float clampedDelta = std::min(deltaTime, 0.05f);
		dynamicsWorld->stepSimulation(clampedDelta * phytime, 8, 1.0f / 60.0f);

		DebugContactCallback callback(dynamicsWorld, deltaTime);
		int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();





		for (int i = 0; i < numManifolds; ++i) {
			btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
			const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
			const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

			int numContacts = contactManifold->getNumContacts();
			for (int j = 0; j < numContacts; ++j) {
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				if (pt.getDistance() < 0.0f) {
					const btVector3& ptA = pt.getPositionWorldOnA();
					const btVector3& ptB = pt.getPositionWorldOnB();

					btScalar impulse = pt.getAppliedImpulse();
					btScalar power = (deltaTime > 0.0f) ? impulse / deltaTime : impulse;

					float voxelSize = 1.0f;




				}
			}
		}


		size_t firstDynamicIndex = GetFirstDynamicBodyIndex();
		for (size_t i = 0; i < dynamicCubeTransforms.size(); ++i) {
			size_t bodyIndex = firstDynamicIndex + i;
			if (bodyIndex >= physicsBodies.size()) break;

			btRigidBody* body = physicsBodies[bodyIndex];
			if (!body || !body->getMotionState()) continue;

			btTransform transform;
			body->getMotionState()->getWorldTransform(transform);


			dynamicCubeTransforms[i].first = glm::vec3(
				transform.getOrigin().getX(),
				transform.getOrigin().getY(),
				transform.getOrigin().getZ()
			);

			btQuaternion rot = transform.getRotation();
			if (rot.length2() > SIMD_EPSILON) {
				glm::vec3 eulerRot;
				rot.getEulerZYX(eulerRot.z, eulerRot.y, eulerRot.x);
				dynamicCubeTransforms[i].second = glm::degrees(eulerRot);
			}
		}
	}


	

	void RemoveCustomShape(size_t shapeId) {
		auto it = customShapes.find(shapeId);
		if (it == customShapes.end()) {
			std::cout << "Warning: Custom shape with ID " << shapeId << " not found for removal" << std::endl;
			return;
		}

		btRigidBody* body = it->second;

		bodyToBatchInfo.erase(body);

		dynamicsWorld->removeRigidBody(body);

		auto bodyIt = std::find(physicsBodies.begin(), physicsBodies.end(), body);
		if (bodyIt != physicsBodies.end()) {
			size_t index = bodyIt - physicsBodies.begin();
			physicsBodies.erase(bodyIt);
			if (index < renderFlags.size()) renderFlags.erase(renderFlags.begin() + index);
		}

		colliderVisualizations.erase(body);

		if (body->getMotionState()) {
			delete body->getMotionState();
		}
		if (body->getCollisionShape()) {
			delete body->getCollisionShape();
		}
		delete body;

		customShapes.erase(it);
		customShapeIds.erase(body);
		customShapeMaterials.erase(body);
		customShapeRenderFlags.erase(body);

		std::cout << "Removed custom shape with ID " << shapeId << std::endl;
	}

	

	bool IsValidBodyIndex(size_t index)  {
		return index < physicsBodies.size() && physicsBodies[index] != nullptr;
	}

	bool HasValidMotionState(size_t index)  {
		return IsValidBodyIndex(index) && physicsBodies[index]->getMotionState() != nullptr;
	}



	const ColliderData* GetColliderData(btRigidBody* body) {
		auto it = colliderVisualizations.find(body);
		if (it != colliderVisualizations.end()) {
			return &it->second;
		}
		return nullptr;
	}


	bool IsBodyDynamic(size_t objectId) {
		btRigidBody* body = GetBodyById(objectId);
		if (!body) return false;


		if (customShapes.find(objectId) != customShapes.end()) {
			return body->getInvMass() != 0.0f;
		}

		auto it = bodyIsDynamic.find(body);
		return it != bodyIsDynamic.end() ? it->second : false;
	}


	const Material& GetMaterial(size_t index) {
		if (index < materials.size()) {
			return materials.at(index);
		}

		static Material fallbackMaterial{ 1.0f, 0.5f };
		return fallbackMaterial;
	}

	const std::vector<std::pair<glm::vec3, glm::vec3>>& GetDynamicCubes() {
		return dynamicCubeTransforms;
	}

	const std::vector<std::pair<glm::vec3, glm::vec3>>& GetStaticCubes()  {
		return staticCubeTransforms;
	}



	const std::vector<btRigidBody*>& GetPhysicsBodies() {
		return physicsBodies;
	}

	size_t GetDynamicCubeCount() {
		return dynamicCubeTransforms.size();
	}

	size_t GetStaticCubeCount() {
		return staticCubeTransforms.size();
	}

	size_t GetFirstDynamicBodyIndex() {
		return staticCubeTransforms.size();
	}
	void AddCustomShapeFromMesh(
		size_t shapeId,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const Material& material,
		bool isDynamic,
		const glm::bvec3& lockRotation,
		bool collider,
		const std::vector<glm::vec3>& vertices,
		const glm::vec3& scale)
	{
		if (vertices.empty()) {
			std::cerr << "Error: No vertices provided for custom shape " << shapeId << std::endl;
			return;
		}

		//collider logic
		btCollisionShape* shape = nullptr;



		btConvexHullShape* convexShape = new btConvexHullShape();

		for (const auto& v : vertices) {


			convexShape->addPoint(btVector3(v.x * scale.x, v.y * scale.y, v.z * scale.z), false);
		}
		convexShape->recalcLocalAabb();
		convexShape->optimizeConvexHull();
		shape = convexShape;



		////

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(position.x, position.y, position.z));
		btQuaternion quat;

		quat.setEulerZYX(
			glm::radians(rotation.z),
			glm::radians(rotation.y),
			glm::radians(rotation.x)
		);
		transform.setRotation(quat);

		btScalar mass = isDynamic ? material.mass : 0.0f;
		btVector3 localInertia(0, 0, 0);
		if (mass != 0.0f) {
			shape->calculateLocalInertia(mass, localInertia);
		}

		btDefaultMotionState* motionState = new btDefaultMotionState(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);


		if (isDynamic) {
			body->setAngularFactor(btVector3(
				lockRotation.x ? 0.0f : 1.0f,
				lockRotation.y ? 0.0f : 1.0f,
				lockRotation.z ? 0.0f : 1.0f
			));
		}

		body->setFriction(material.friction);
		//body->setRestitution(0.05f);
		body->setCcdMotionThreshold(0.0001f);
		body->setCcdSweptSphereRadius(0.2f);

		if (!isDynamic) {
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
		}
		else {
			body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT);
			body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
		}

		dynamicsWorld->addRigidBody(body);
		std::cout << "rigid body added " << std::endl;

		customShapes[shapeId] = body;
		customShapeIds.insert(body);
		customShapeMaterials[body] = material;



		physicsBodies.push_back(body);


		std::cout << "DEBUG: Created shape " << shapeId << " at position: "
			<< position.x << ", " << position.y << ", " << position.z << std::endl;
	}


	void applyForceToPhysicsId(size_t physId, const glm::vec3& force) {

		btRigidBody* body = GetBodyById(physId);
		if (!body) return;

		body->activate(true);
		body->applyCentralImpulse(btVector3(force.x, force.y, force.z));
	}

	BodyInfo GetBodyInfoById(size_t objectId) {
		BodyInfo info;

		btRigidBody* body = GetBodyById(objectId);
		if (!body) {
			return info;
		}

		btTransform transform;
		body->getMotionState()->getWorldTransform(transform);


		btVector3 pos = transform.getOrigin();
		info.position = glm::vec3(pos.x(), pos.y(), pos.z());

		btQuaternion rot = transform.getRotation();

		info.quatRotation = glm::quat(rot.getX(), rot.getY(), rot.getZ(), rot.getW());
		if (rot.length2() > SIMD_EPSILON) {
			glm::vec3 euler;
			rot.getEulerZYX(euler.z, euler.y, euler.x);
			info.rotation = glm::degrees(euler);
		}

		auto matIt = customShapeMaterials.find(body);
		if (matIt != customShapeMaterials.end()) {
			info.material = matIt->second;
		}
		else {
			auto idIt = bodyToId.find(body);
			if (idIt != bodyToId.end()) {
				size_t index = idIt->second;
				if (index < materials.size()) {
					info.material = materials[index];
				}
			}
		}


		info.isDynamic = IsBodyDynamic(objectId);

		btVector3 angFactor = body->getAngularFactor();
		info.lockRotation = glm::bvec3(
			angFactor.x() == 0.0f,
			angFactor.y() == 0.0f,
			angFactor.z() == 0.0f
		);

		info.found = true;
		return info;
	}
	void AddHingeConstraint(
		size_t objectA,
		size_t objectB,
		const btVector3& worldHingePos,
		const btVector3& worldHingeAxis,
		bool disableCollisionsBetweenLinkedBodies
	)
	{
		btRigidBody* bodyA = GetBodyById(objectA);
		btRigidBody* bodyB = GetBodyById(objectB);

		if (!bodyA || !bodyB) {
			std::cerr << "Error: Invalid bodies for hinge constraint." << std::endl;
			return;
		}



		btVector3 comA = bodyA->getCenterOfMassPosition();
		btVector3 comB = bodyB->getCenterOfMassPosition();

		std::cout << "comAx" << comA.getX() << std::endl;
		btVector3 pivotInA = worldHingePos - comA;
		btVector3 pivotInB = worldHingePos - comB;


		btMatrix3x3 basisA, basisB;

		btVector3 xAxis = worldHingeAxis.normalized();
		btVector3 yAxis(0, 1, 0);
		if (fabs(xAxis.dot(yAxis)) > 0.99) yAxis = btVector3(0, 0, 1);
		btVector3 zAxis = xAxis.cross(yAxis).normalized();
		yAxis = zAxis.cross(xAxis).normalized();

		basisA[0] = xAxis; basisA[1] = yAxis; basisA[2] = zAxis;
		basisB = basisA;

		btTransform frameA, frameB;
		frameA.setIdentity();
		frameA.setOrigin(pivotInA);
		frameA.setBasis(basisA);

		frameB.setIdentity();
		frameB.setOrigin(pivotInB);
		frameB.setBasis(basisB);

		btHingeConstraint* hinge = new btHingeConstraint(*bodyA, *bodyB, frameA, frameB);

		dynamicsWorld->addConstraint(hinge, disableCollisionsBetweenLinkedBodies);
		constraints.push_back(hinge);

		glm::vec3 hingePos(worldHingePos.getX(), worldHingePos.getY(), worldHingePos.getZ());
		hingePositions.push_back(hingePos);

		std::cout << "Added hinge between bodies " << objectA << " and " << objectB
			<< " at " << hingePos.x << ", " << hingePos.y << ", " << hingePos.z
			<< std::endl;
	}

	std::vector<std::pair<btRigidBody*, btRigidBody*>> GetHighImpactCollisions(float threshold) {
		ImpactDetectionCallback callback(threshold);

		for (btRigidBody* body : physicsBodies) {
			if (!body) continue;
			dynamicsWorld->contactTest(body, callback);
		}

		return callback.detectedCollisions;
	}

	void RemoveBody(btRigidBody* body) {
		if (!body) return;

		dynamicsWorld->removeRigidBody(body);

		bodyIsDynamic.erase(body);
		bodyToId.erase(body);
		bodyToBatchInfo.erase(body);
		colliderVisualizations.erase(body);
		customShapeIds.erase(body);
		customShapeMaterials.erase(body);
		customShapeRenderFlags.erase(body);

		size_t foundCustomId = SIZE_MAX;
		for (const auto& p : customShapes) {
			if (p.second == body) {
				foundCustomId = p.first;
				break;
			}
		}

		if (foundCustomId != SIZE_MAX) {
			customShapes.erase(foundCustomId);
			colliderWorldVerticesById.erase(foundCustomId);
			colliderWorldVerticesByIdTest.erase(foundCustomId);
		}

		auto it = std::find(physicsBodies.begin(), physicsBodies.end(), body);
		if (it != physicsBodies.end()) {
			physicsBodies.erase(it);
		}



		if (body->getMotionState()) delete body->getMotionState();
		if (body->getCollisionShape()) delete body->getCollisionShape();

		delete body;
	}
	void deletebyId(size_t objectId) {
		std::cout << "Deleting object with ID: " << objectId << std::endl;

		auto customIt = customShapes.find(objectId);
		if (customIt != customShapes.end()) {
			btRigidBody* body = customIt->second;
			std::cout << "Found custom shape with ID: " << objectId << std::endl;

			dynamicsWorld->removeRigidBody(body);

			customShapes.erase(customIt);
			customShapeIds.erase(body);
			customShapeMaterials.erase(body);
			customShapeRenderFlags.erase(body);
			colliderWorldVerticesById.erase(objectId);
			colliderWorldVerticesByIdTest.erase(objectId);

			auto bodyIt = std::find(physicsBodies.begin(), physicsBodies.end(), body);
			if (bodyIt != physicsBodies.end()) {
				size_t index = bodyIt - physicsBodies.begin();
				physicsBodies.erase(bodyIt);

				if (index < renderFlags.size()) {
					renderFlags.erase(renderFlags.begin() + index);
				}
			}

			if (body->getMotionState()) {
				delete body->getMotionState();
			}
			if (body->getCollisionShape()) {
				delete body->getCollisionShape();
			}
			delete body;

			std::cout << "Successfully deleted custom shape with ID: " << objectId << std::endl;
			return;
		}

		auto cubeIt = idToBodies.find(objectId);
		if (cubeIt != idToBodies.end() && !cubeIt->second.empty()) {
			std::cout << "Found regular cube with ID: " << objectId << " ("
				<< cubeIt->second.size() << " bodies)" << std::endl;

			for (btRigidBody* body : cubeIt->second) {
				if (!body) continue;

				dynamicsWorld->removeRigidBody(body);

				bodyToId.erase(body);
				bodyIsDynamic.erase(body);
				bodyToBatchInfo.erase(body);
				colliderVisualizations.erase(body);

				auto bodyIt = std::find(physicsBodies.begin(), physicsBodies.end(), body);
				if (bodyIt != physicsBodies.end()) {
					size_t index = bodyIt - physicsBodies.begin();
					physicsBodies.erase(bodyIt);

					if (index < materials.size()) {
						materials.erase(materials.begin() + index);
					}
					if (index < renderFlags.size()) {
						renderFlags.erase(renderFlags.begin() + index);
					}

					size_t firstDynamicIndex = GetFirstDynamicBodyIndex();
					if (index < firstDynamicIndex) {
						size_t staticIndex = index;
						if (staticIndex < staticCubeTransforms.size()) {
							staticCubeTransforms.erase(staticCubeTransforms.begin() + staticIndex);
						}
					}
					else {
						size_t dynamicIndex = index - firstDynamicIndex;
						if (dynamicIndex < dynamicCubeTransforms.size()) {
							dynamicCubeTransforms.erase(dynamicCubeTransforms.begin() + dynamicIndex);
						}
					}
				}

				if (body->getMotionState()) {
					delete body->getMotionState();
				}
				if (body->getCollisionShape()) {
					delete body->getCollisionShape();
				}
				delete body;
			}


			idToBodies.erase(cubeIt);
			std::cout << "Successfully deleted regular cube with ID: " << objectId << std::endl;
			return;
		}

		std::cout << "Warning: Object with ID " << objectId << " not found for deletion" << std::endl;
	}




	void phytimeScale(float time)
	{
		phytime = time;
	}
	void phyGravityScale(float gravity)
	{
		dynamicsWorld->setGravity(btVector3(0, -gravity, 0));
	}


	void SetColliderEnabled(size_t shapeId, bool enabled)
	{
		auto it = customShapes.find(shapeId);
		if (it == customShapes.end()) {
			std::cerr << "SetColliderEnabled ERROR: No shape with ID " << shapeId << std::endl;
			return;
		}

		btCollisionObject* obj = it->second;
		int flags = obj->getCollisionFlags();

		if (enabled) {

			flags &= ~btCollisionObject::CF_NO_CONTACT_RESPONSE;
			obj->setCollisionFlags(flags);

			std::cout << "DEBUG: Collider ENABLED for shape " << shapeId << std::endl;
		}
		else {

			flags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
			obj->setCollisionFlags(flags);

			std::cout << "DEBUG: Collider DISABLED for shape " << shapeId << std::endl;
		}
	}
	void SetRigidbodyEnabled(size_t shapeId, bool enabled)
	{
		auto it = customShapes.find(shapeId);
		if (it == customShapes.end()) {
			std::cerr << "SetRigidbodyEnabled ERROR: No shape with ID " << shapeId << std::endl;
			return;
		}

		btRigidBody* body = it->second;

		if (enabled) {

			if (!body->isInWorld())
			{
				dynamicsWorld->addRigidBody(body);

				std::cout << "DEBUG: Rigidbody ENABLED for shape " << shapeId << std::endl;
			}
		}
		else {

			if (body->isInWorld())
			{
				dynamicsWorld->removeRigidBody(body);

				std::cout << "DEBUG: Rigidbody DISABLED for shape " << shapeId << std::endl;
			}
		}
	}




	void PushObjectsFromPositionEnhanced(
		const glm::vec3& position,
		float radius,
		float forceStrength,
		bool useRadialForce,
		bool useUpwardForce,
		float upwardForceRatio,
		bool useImpulse,
		ForceFalloff falloff
	) {
		float radiusSq = radius * radius;


		auto applyForceToBody = [&](btRigidBody* body, size_t id = SIZE_MAX, bool isCustom = false) {
			if (!body) return;

			btTransform transform;
			if (body->getMotionState()) {
				body->getMotionState()->getWorldTransform(transform);
			}
			else {
				transform = body->getWorldTransform();
			}

			btVector3 btBodyPos = transform.getOrigin();
			glm::vec3 bodyPos(btBodyPos.x(), btBodyPos.y(), btBodyPos.z());

			float dx = bodyPos.x - position.x;
			float dy = bodyPos.y - position.y;
			float dz = bodyPos.z - position.z;
			float distanceSq = dx * dx + dy * dy + dz * dz;

			if (distanceSq <= radiusSq && distanceSq > 0.001f) {
				float distance = std::sqrt(distanceSq);
				glm::vec3 direction = glm::normalize(bodyPos - position);

				float forceMagnitude;
				switch (falloff) {
				case ForceFalloff::CONSTANT:
					forceMagnitude = forceStrength;
					break;
				case ForceFalloff::LINEAR:
					forceMagnitude = forceStrength * (1.0f - (distance / radius));
					break;
				case ForceFalloff::INVERSE_SQUARE:
					forceMagnitude = forceStrength / std::max(distance * distance, 0.1f);
					break;
				default:
					forceMagnitude = forceStrength;
				}

				glm::vec3 force(0.0f);

				if (useRadialForce) {
					force += direction * forceMagnitude;
				}

				if (useUpwardForce) {
					force.y += forceMagnitude * upwardForceRatio;
				}


				if (glm::length(force) < 0.01f && forceMagnitude > 0) {
					force = glm::vec3(0, forceMagnitude * 0.1f, 0);
				}

				body->activate(true);

				if (useImpulse) {
					body->applyCentralImpulse(btVector3(force.x, force.y, force.z));
				}
				else {
					body->applyCentralForce(btVector3(force.x, force.y, force.z));
				}

				/*
							std::cout << "  Pushing " << (isCustom ? "custom shape" : "regular body") << " ";
				if (id != SIZE_MAX) std::cout << "ID " << id;
				std::cout << " with force (" << force.x << ", " << force.y << ", " << force.z << ")"
					<< " distance: " << distance
					<< " magnitude: " << forceMagnitude << std::endl;
				*/

			}
			};

		for (const auto& [shapeId, body] : customShapes) {
			applyForceToBody(body, shapeId, true);
		}

		for (size_t i = 0; i < physicsBodies.size(); ++i) {
			btRigidBody* body = physicsBodies[i];
			if (!body) continue;


			if (customShapeIds.find(body) != customShapeIds.end()) {
				continue;
			}

			size_t id = SIZE_MAX;
			auto it = bodyToId.find(body);
			if (it != bodyToId.end()) id = it->second;

			applyForceToBody(body, id, false);
		}
	}




	RaycastHit RaycastPhysics(
		const glm::vec3& origin,
		const glm::vec3& direction,
		float maxDistance)
	{
		RaycastHit result;

		glm::vec3 normDir = glm::normalize(direction);
		glm::vec3 endPoint = origin + normDir * maxDistance;

		btVector3 btFrom(origin.x, origin.y, origin.z);
		btVector3 btTo(endPoint.x, endPoint.y, endPoint.z);

		btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);

		dynamicsWorld->rayTest(btFrom, btTo, rayCallback);

		if (!rayCallback.hasHit())
			return result;

		result.hit = true;

		btVector3 hp = rayCallback.m_hitPointWorld;
		btVector3 hn = rayCallback.m_hitNormalWorld;

		result.hitPoint = glm::vec3(hp.x(), hp.y(), hp.z());
		result.hitNormal = glm::vec3(hn.x(), hn.y(), hn.z());
		result.distance = glm::length(result.hitPoint - origin);

		const btCollisionObject* hitObj = rayCallback.m_collisionObject;
		result.body = const_cast<btRigidBody*>(btRigidBody::upcast(hitObj));

		if (result.body) {
			for (const auto& [id, body] : customShapes) {
				if (body == result.body) {
					result.objectId = id;
					return result;
				}
			}

			auto it = bodyToId.find(result.body);
			if (it != bodyToId.end())
				result.objectId = it->second;
		}

		return result;
	}

	

	void RemoveBodyById(size_t id) {
		auto it = customShapes.find(id);
		if (it == customShapes.end()) return;

		btRigidBody* body = it->second;
		dynamicsWorld->removeRigidBody(body);

		colliderVisualizations.erase(body);
		colliderWorldVerticesById.erase(id);
		customShapeIds.erase(body);
		customShapeMaterials.erase(body);
		customShapeRenderFlags.erase(body);

		physicsBodies.erase(
			std::remove(physicsBodies.begin(), physicsBodies.end(), body),
			physicsBodies.end()
		);

		delete body->getMotionState();
		delete body->getCollisionShape();
		delete body;

		customShapes.erase(it);
	}

	void createMotor(
		size_t objectA,
		size_t objectB,
		const btVector3& worldHingePos,
		const btVector3& worldHingeAxis,
		bool disableCollisionsBetweenLinkedBodies,
		float throttle,
		float maxSpeed,
		float strength,
		float dt
	)
	{
		btRigidBody* bodyA = GetBodyById(objectA);
		btRigidBody* bodyB = GetBodyById(objectB);

		if (!bodyA || !bodyB) {
			std::cerr << "Error: Invalid bodies for hinge constraint." << std::endl;
			return;
		}



		btVector3 comA = bodyA->getCenterOfMassPosition();
		btVector3 comB = bodyB->getCenterOfMassPosition();


		std::cout << "comAx" << comA.getX() << std::endl;
		btVector3 pivotInA = worldHingePos - comA;
		btVector3 pivotInB = worldHingePos - comB;


		btMatrix3x3 basisA, basisB;

		btVector3 xAxis = worldHingeAxis.normalized();
		btVector3 yAxis(0, 1, 0);
		if (fabs(xAxis.dot(yAxis)) > 0.99) yAxis = btVector3(0, 0, 1);
		btVector3 zAxis = xAxis.cross(yAxis).normalized();
		yAxis = zAxis.cross(xAxis).normalized();

		basisA[0] = xAxis; basisA[1] = yAxis; basisA[2] = zAxis;
		basisB = basisA;

		btTransform frameA, frameB;
		frameA.setIdentity();
		frameA.setOrigin(pivotInA);
		frameA.setBasis(basisA);

		frameB.setIdentity();
		frameB.setOrigin(pivotInB);
		frameB.setBasis(basisB);

		btHingeConstraint* hinge = new btHingeConstraint(*bodyA, *bodyB, frameA, frameB);

		hinge->enableAngularMotor(
			true,
			throttle * maxSpeed,
			strength
		);
		hinge->enableMotor(true);

		motors[objectB] = hinge;
		dynamicsWorld->addConstraint(hinge, disableCollisionsBetweenLinkedBodies);
		constraints.push_back(hinge);

		glm::vec3 hingePos(worldHingePos.getX(), worldHingePos.getY(), worldHingePos.getZ());
		hingePositions.push_back(hingePos);

		std::cout << "Added hinge between bodies " << objectA << " and " << objectB
			<< " at " << hingePos.x << ", " << hingePos.y << ", " << hingePos.z
			<< std::endl;
	}
	void ApplyTorqueToShape(size_t Index, const glm::vec3& torque) {

		btRigidBody* body = GetBodyById(Index);

		if (!body) return;



		btVector3 worldTorque = btVector3(torque.x, torque.y, torque.z);
		body->activate(true);
		body->applyTorque(worldTorque);
	}
	void ApplyForceToShape(size_t Index, const glm::vec3& force) {


		btRigidBody* body = GetBodyById(Index);
		if (!body || !body->isActive()) return;

		body->activate(true);
		body->applyCentralForce(btVector3(force.x, force.y, force.z));
	}


	void fixedConstraint(
		size_t objectA,
		size_t objectB,
		bool disableCollisionsBetweenLinkedBodies
	)
	{
		btRigidBody* bodyA = GetBodyById(objectA);
		btRigidBody* bodyB = GetBodyById(objectB);

		if (!bodyA || !bodyB) {
			std::cerr << "Error: Invalid bodies for hinge constraint." << std::endl;
			return;
		}


		btMatrix3x3 basisA, basisB;

		btTransform worldFrame;
		worldFrame = (bodyA->getCenterOfMassTransform());

		btTransform frameA, frameB;
		frameA = bodyA->getCenterOfMassTransform().inverse() * worldFrame;
		frameB = bodyB->getCenterOfMassTransform().inverse() * worldFrame;



		btFixedConstraint* attachment = new btFixedConstraint(*bodyA, *bodyB, frameA, frameB);


		dynamicsWorld->addConstraint(attachment, disableCollisionsBetweenLinkedBodies);
		constraints.push_back(attachment);


	}

	void setMotorSpeed(size_t id, float speed, float maxImpulse)
	{
		auto it = motors.find(id);

		if (it == motors.end())
			return;

		btHingeConstraint* hinge = it->second;

		hinge->enableAngularMotor(true, speed, maxImpulse);

	}


	void forceToward(size_t physId, const glm::vec3& targetPos, float strength) {

		btRigidBody* body = GetBodyById(physId);
		if (!body) return;


		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
		btVector3 btPos = trans.getOrigin();
		glm::vec3 currentPos(btPos.x(), btPos.y(), btPos.z());

		glm::vec3 diff = targetPos - currentPos;
		if (glm::length(diff) < 0.001f) return;

		glm::vec3 force = diff * strength;

		body->activate(true);
		btVector3 vel = body->getLinearVelocity();
		body->setLinearVelocity(vel * 0.7f);
		body->applyCentralForce(btVector3(force.x, force.y, force.z));
	}


}