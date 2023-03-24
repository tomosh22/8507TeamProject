#include "Paintball.h"
#include "PhysicsObject.h"

#include <algorithm>

namespace NCL::CSC8503 {
	
	Paintball::Paintball(float paintRadius, float collisionRadius, uint8_t teamId) :
		paintRadius(paintRadius), collisionRadius(collisionRadius), teamId(teamId),
		moveSpeed(200), artificalGravity()
	{
		
	}

	Paintball::~Paintball()
	{

	}

	void Paintball::Update(float dt)
	{
		lifeTime += dt;
		if (lifeTime >= 0.1) GetPhysicsObject()->SetLinearVelocity(GetPhysicsObject()->GetLinearVelocity() + Vector3(0,-1,0) * (dt * 50.0f));
		
		GetTransform().SetPosition(GetTransform().GetPosition() + forward * (dt * moveSpeed));
	}

	void Paintball::OnCollisionBegin(GameObject* object)
	{
		//Ignore the collision if the other object is a paintball
		if (dynamic_cast<Paintball*>(object)) return;

		hasHit = true;
	}

}
