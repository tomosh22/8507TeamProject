#pragma once
#include "GameWorld.h"
#include "GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem {
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();
			struct collisionData
			{
				float paintRadius;
				Vector3 contactPosition;
				GameObject* objectHit;

			};

			std::vector <collisionData> getCurrentCollisions() {
				return currentCollisions;
			}

			void clearCurrentCollisions() {
				currentCollisions = {};
			}

			void AddToCurrentCollision(collisionData nextSphereData) {
				if (nextSphereData.paintRadius == 0.0f) {
					return;
				}
				currentCollisions.push_back(nextSphereData);
			}


			void Clear();

			void Update(float dt);
			void UseGravity(bool state) {
				applyGravity = state;
			}
			void SetGlobalDamping(float d) {
				globalDamping = d;
			}
			void SetGravity(const Vector3& g);

		protected:
			void BasicCollisionDetection(float timeIncrement);
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();
			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);
			void UpdateConstraints(float dt);
			void UpdateCollisionList();
			void UpdateObjectAABBs();
			void ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void ImpulseResolveStop(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void ImpulseResolveContinuedResponse(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			GameWorld& gameWorld;

			bool	applyGravity;
			Vector3 gravity;
			std::vector <collisionData> currentCollisions;
			float	dTOffset;
			float	globalDamping;
			GameObject* hitObject;

			std::set<CollisionDetection::CollisionInfo> allCollisions;
			std::set<CollisionDetection::CollisionInfo> broadphaseCollisions;
			std::vector<CollisionDetection::CollisionInfo> broadphaseCollisionsVec;
			bool useBroadPhase = true;
			int numCollisionFrames = 5;
		};
	}
}
