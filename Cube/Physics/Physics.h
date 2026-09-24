#pragma once
#include "Dependencies.h"
#include "Material.h"
#include "Camera.h"

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif


namespace Cube::Physics {



	enum class CubeType {
		Static,
		Dynamic,
		Both
	};


	struct BodyInfo {
		glm::vec3 position;
		glm::vec3 rotation;
		Cube::Material material;
		bool isDynamic;
		glm::bvec3 lockRotation;
		bool found = false;
		glm::quat quatRotation = glm::quat(1,1,1,1);
	};

	struct VoxelHit {
		btRigidBody* voxelBody;
		glm::ivec3 voxel;
		btVector3 worldPoint;
		btVector3 normal;
	};



	struct ColliderData {
		std::vector<glm::vec3> vertices;
		std::vector<uint16_t> indices;
	};

	inline std::unordered_map<size_t, std::vector<glm::vec3>> colliderWorldVerticesById;
	inline std::unordered_map<size_t, std::vector<glm::vec3>> colliderWorldVerticesByIdTest;
	inline std::vector<btRigidBody*> physicsBodies;

	void intialize();

	void applyForceToPhysicsId(size_t physId, const glm::vec3& force);

	void AddHingeConstraint(
		size_t objectA,
		size_t objectB,
		const btVector3& worldHingePos,
		const btVector3& worldHingeAxis,
		bool disableCollisionsBetweenLinkedBodies
	);
	inline std::vector<glm::vec3> hingePositions;
	inline const std::vector<glm::vec3>& GetHingePositions()  { return hingePositions; }
	BodyInfo GetBodyInfoById(size_t objectId) ;

	struct HingeDebug {
		glm::vec3 position;
		glm::vec3 color;
		float width, height, depth;
	};

	inline std::vector<HingeDebug> hingeDebugPositions;

	void deletebyId(size_t objectId);

	void forceToward(size_t physId, const glm::vec3& targetPos, float strength);

	void RemoveBody(btRigidBody* body);
	size_t positiontoobject(glm::vec3& position);
	float distance(glm::vec3& position1, glm::vec3& position2);
	glm::vec3 bulletToGlm(const btVector3& v);
	btVector3 glmToBullet(const glm::vec3& v);
	void phytimeScale(float time);
	void phyGravityScale(float grav);



	void fixedConstraint(size_t objectA, size_t objectB, bool disableCollisionsBetweenLinkedBodies);
	void createMotor(size_t objectA,
		size_t objectB,
		const btVector3& worldHingePos,
		const btVector3& worldHingeAxis,
		bool disableCollisionsBetweenLinkedBodies,
		float throttle,
		float maxSpeed,
		float strength,
		float dt);


	inline float phytime = 1.0f;

	void RemoveBodyById(size_t id);

	void ApplyTorqueToShape(size_t index, const glm::vec3& torque);
	void ApplyForceToShape(size_t index, const glm::vec3& force);

	inline std::unordered_map<size_t, btHingeConstraint*> motors;
	void setMotorSpeed(size_t id, float speed, float maxImpulse);

	//void combineBodies(size_t objectA, size_t objectB);
	inline float gravity = -9.81f;



	inline bool fiveKeyPressed = false;
	struct ImpactDetectionCallback : public btCollisionWorld::ContactResultCallback {
		float minSpeed;
		std::vector<std::pair<btRigidBody*, btRigidBody*>> detectedCollisions;

		ImpactDetectionCallback(float threshold) : minSpeed(threshold) {}

		virtual btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap,
			int partId0, int index0,
			const btCollisionObjectWrapper* colObj1Wrap,
			int partId1, int index1) override
		{
			btRigidBody* bodyA = (btRigidBody*)colObj0Wrap->getCollisionObject();
			btRigidBody* bodyB = (btRigidBody*)colObj1Wrap->getCollisionObject();

			btVector3 relativeVel = bodyA->getLinearVelocity() - bodyB->getLinearVelocity();
			if (relativeVel.length() > minSpeed) {
				detectedCollisions.emplace_back(bodyA, bodyB);
			}
			return 0;
		}
	};

	std::vector<std::pair<btRigidBody*, btRigidBody*>> GetHighImpactCollisions(float threshold);
	
	inline int iterations = 0;

	struct BatchInfo {
		int batchIndex = -1;
		bool hasBatch = false;
	};
	inline std::unordered_map<btRigidBody*, BatchInfo> bodyToBatchInfo;

	inline std::unordered_map<btRigidBody*, ColliderData> colliderVisualizations;


	inline std::unordered_map<size_t, btRigidBody*> customShapes;
	inline std::unordered_set<btRigidBody*> customShapeIds;
	inline std::unordered_map<btRigidBody*, Material> customShapeMaterials;
	inline std::unordered_map<btRigidBody*, bool> customShapeRenderFlags;


	inline std::unordered_map<size_t, std::vector<btRigidBody*>> cubesById;

	inline std::unordered_map<btRigidBody*, size_t> bodyToId;

	inline std::vector<std::pair<glm::vec3, glm::vec3>> dynamicCubeTransforms;
	inline std::vector<std::pair<glm::vec3, glm::vec3>> staticCubeTransforms;
	inline std::vector<btRigidBody*> terrainRigidBodies;



	inline std::vector<Material> materials;
	inline std::unordered_map<btRigidBody*, bool> bodyIsDynamic;


	inline btDiscreteDynamicsWorld* dynamicsWorld;
	inline btBroadphaseInterface* broadphase;
	inline btDefaultCollisionConfiguration* collisionConfig;
	inline btCollisionDispatcher* dispatcher;
	inline btSequentialImpulseConstraintSolver* solver;



	inline std::vector<btTypedConstraint*> constraints;


	enum class ForceFalloff {
		CONSTANT,
		LINEAR,
		INVERSE_SQUARE
	};
	inline std::unordered_map<size_t, std::vector<btRigidBody*>> idToBodies;

	
	void PushObjectsFromPositionEnhanced(
		const glm::vec3& position,
		float radius,
		float forceStrength,
		bool useRadialForce = true,
		bool useUpwardForce = false,
		float upwardForceRatio = 0.3f,
		bool useImpulse = false,
		ForceFalloff falloff = ForceFalloff::LINEAR
	);
	struct RaycastHit {
		bool hit = false;
		size_t objectId = SIZE_MAX;
		glm::vec3 hitPoint = glm::vec3(0.0f);
		glm::vec3 hitNormal = glm::vec3(0.0f);
		float distance = 0.0f;
		btRigidBody* body = nullptr;
	};

	RaycastHit RaycastPhysics(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 500.0f);

	void SetRigidbodyEnabled(size_t shapeId, bool enabled);
	void SetColliderEnabled(size_t shapeId, bool enabled);
	
	



	void AddCustomShapeFromMesh(
		size_t shapeId,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const Material& material,
		bool isDynamic,
		const glm::bvec3& lockRotation,
		bool render,
		const std::vector<glm::vec3>& vertices,
		const glm::vec3& scale = glm::vec3(1.0f)
	);
	




	inline const std::unordered_map<btRigidBody*, ColliderData>& GetColliderVisualizations() {
		return colliderVisualizations;
	}

	

	const ColliderData* GetColliderData(btRigidBody* body);

	inline std::vector<Material> physicsMaterials;


	void RemoveCustomShape(size_t shapeId);
	inline btRigidBody* GetBodyById(size_t objectId) {

		auto customIt = customShapes.find(objectId);
		if (customIt != customShapes.end()) {
			return customIt->second;
		}

		auto cubeIt = idToBodies.find(objectId);
		if (cubeIt != idToBodies.end() && !cubeIt->second.empty()) {
			return cubeIt->second[0];
		}

		return nullptr;
	}
	bool IsBodyDynamic(size_t objectId);

	void Update(float deltaTime);


	void AddCube(size_t cubeId,
		const glm::vec3& position,
		const glm::vec3& rotation,
		const Material& material,
		bool isDynamic,
		const glm::bvec3& lockRotation = glm::bvec3(false),
		bool render = true);

	void RemoveCubeById(size_t cubeId, btRigidBody* playerBody);


	inline void AddStaticBody(btRigidBody* body) {
		dynamicsWorld->addRigidBody(body);
	}

	inline void AddBody(btRigidBody* body) {
		dynamicsWorld->addRigidBody(body);
	}

	void RemoveAllBodies();


	const std::vector<std::pair<glm::vec3, glm::vec3>>& GetDynamicCubes();
	const std::vector<std::pair<glm::vec3, glm::vec3>>& GetStaticCubes();

	const Material& GetMaterial(size_t index);

	const std::vector<btRigidBody*>& GetPhysicsBodies();

	bool IsValidBodyIndex(size_t index);
	bool HasValidMotionState(size_t index);
	size_t GetDynamicCubeCount();
	size_t GetStaticCubeCount();
	size_t GetFirstDynamicBodyIndex();

	inline std::vector<bool> renderFlags; // tracks cubes to render

	inline const std::vector<bool>& GetRenderFlags() {
		return renderFlags;
	}


	inline btDiscreteDynamicsWorld* GetDynamicsWorld() { return dynamicsWorld; }
	inline const std::vector<Material>& GetMaterials() { return materials; }

	inline void AddTerrainBody(btRigidBody* body) {
		dynamicsWorld->addRigidBody(body);
		terrainRigidBodies.push_back(body);
	}

	inline const std::vector<btRigidBody*>& GetTerrainBodies()  {
		return terrainRigidBodies;
	}


};
