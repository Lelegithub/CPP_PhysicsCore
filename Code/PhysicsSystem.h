#ifndef _H_PHYSICS_SYSTEM_REFACTORED_
#define _H_PHYSICS_SYSTEM_REFACTORED_

#include "RigidBody.h"
#include "ElasticSpring.h"
#include "Fabric.h"

class PhysicsSystemRefactored {
protected:
	std::vector<RigidBody*> dynamicBodies;
	std::vector<Fabric*> fabricSimulations;
	std::vector<AABB> collisionConstraints;
	std::vector<ElasticSpring> elasticSprings;

	std::vector<RigidBody*> collidingObjects1;
	std::vector<RigidBody*> collidingObjects2;
	std::vector<CollisionContact> collisionResults;
public:
	float LinearProjectionFactor; // [0.2 to 0.8], Smaller = less jitter / more penetration
	float PenetrationThreshold;   // [0.01 to 0.1], Smaller = more accuracy
	int ImpulseResolutionIterations;

	// Debug settings (not in the original book code)
	bool EnableDebugRendering;
	bool ApplyLinearProjection;
	bool UseRandomColorsForRendering;

	PhysicsSystemRefactored();

	void UpdateSimulation(float deltaTime);
	void RenderScene();
    
	void AddDynamicBody(RigidBody* body);
	void AddFabricSimulation(Fabric* cloth);
	void AddElasticSpring(const ElasticSpring& spring);
	void AddCollisionConstraint(const AABB& constraint);

	void ClearDynamicBodies();
	void ClearCollisionConstraints();
	void ClearElasticSprings();
	void ClearFabricSimulations();
};

#endif