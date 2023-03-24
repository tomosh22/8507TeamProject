#pragma once
#include "GameObject.h"

namespace NCL::CSC8503 {

	class Paintball : public GameObject
	{
	public:
		Paintball(float paintRadius, float collisionRadius, uint8_t teamId);
		~Paintball();

		void Update(float dt);
		void OnCollisionBegin(GameObject* object);
	public:
		Vector3 forward;
		Vector3 artificalGravity;
		float moveSpeed = 0.0;
		float lifeTime = 0.0;
		bool hasHit = false;

		float paintRadius = 0.0f;
		float collisionRadius = 0.0f;
		uint8_t teamId = 0;
	};

}
