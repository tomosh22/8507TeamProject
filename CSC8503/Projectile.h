#pragma once

#include "Transform.h"
#include "CollisionVolume.h"
#include"GameObject.h"
#include"GameWorld.h"


namespace NCL {
	namespace CSC8503 {
		const enum GunType {
			GUN_TYPE_PISTOL,
			GUN_TYPE_ROCKET,
			GUN_TYPE_MAX
		};

		struct Gun {
			bool affectedByGravity;
			int type;
			float radius;  //exlopsion radius 
			float ProjectileSize; // size of bullet
			float projectileForce; // Fire force
			float weight;// keep a >0 weight on instances. Otherwise applied forces will have no effect //Do not exceed 1.0, otherwise additional forces will be generated
			float rateOfFire; // time in seconds between consecuative bullets shot
		};

		static Gun pistol{
			true,
			GUN_TYPE_PISTOL,
			10.0f,
			0.2f,
			8000.0f,
			1.0f,
			0.7f,
		};

		static Gun rocket{
			true,
			GUN_TYPE_ROCKET,
			6.0f,
			0.5f,
			3000.0f,
			0.7f,
			0.5f,
		};


		class NetworkObject;
		class RenderObject;
		class PhysicsObject;
		class TutorialGame;
		class CollisionInfo;
		class playerTracking;
		class Projectile :public GameObject {
		public:

			/*	Projectile() {
					setGunType(pistol);
					aimingPitch = 0.0f;
					aimingYaw = 0.0f;
					physicsProjectile = nullptr;
				}*/

			Projectile();
			Projectile(Gun GunToUse);
			Projectile(Vector3& position);
			//Projectile(gun GunToUse, vector<Projectile*>* parentVector, GameWorld* world) = 0;
			~Projectile() {
				//physicsProjectile;
			}

			void Update(float dt);
			void setGunType(Gun wepType);


			void DestroySelf() {
				std::vector<Projectile*>::iterator it = parentVector->begin();
				while (it != parentVector->end()) {
					if ((*it) == this) {
						it = parentVector->erase(it);
					}
					else it++;
				}
				world->RemoveGameObject(this, true);
			}

			void setExplosionRadius(float newRadius) {
				explosionRadius = newRadius;
			}

			float getExplosionRadius() {
				return explosionRadius;
			}

			void setProjectileRadius(float newProjectileRadius) {
				ProjectileRadius = newProjectileRadius;
			}

			float getProjectileRadius() {
				return ProjectileRadius;
			}

			void setProjectilePropultionForce(float newForce) {
				projectilePropultionForce = newForce;
			}
			float getPojectilePropultionForce() {
				return projectilePropultionForce;
			}

			void setWieght(float newweight) {
				weight = newweight;
			}

			float getWeight() {
				return weight;
			}

			void setAffectedByGravityTrue() {
				AffectedGravity = true;
			}

			void setAffectedByGravityFalse() {
				AffectedGravity = false;
			}

			void setBulletDirectionVector(Vector3 aimedDirection) {
				bulletDirectionVector = aimedDirection.Normalised();
			}

			Vector3 getBulletDirectionVector() {
				return bulletDirectionVector;
			}

			bool ProjectileAffectedByGravity() {
				return AffectedGravity;
			}

			bool GetCanFire() {
				return canFire;
			}

			void toggleCanFire() {
				canFire = (!canFire);
			}

			void setRateOfFireTransferred(float newRateOfFire) {
				rateOfFireTransferred = newRateOfFire;
			}

			float GetRateOfFireTransferred() {
				return rateOfFireTransferred;
			}

			void OnCollisionBegin(GameObject* otherObject);


			void OnCollisionEnd(GameObject* otherObject);


			float collisionInfo() override {
				return this->getExplosionRadius();
			}

			void SetPlayer(playerTracking* player)
			{
				this->player = player;
			}

			void SetTeamID(int id)
			{
				this->teamID = id;
			}
		protected:
			float explosionRadius;
			float ProjectileRadius;
			float projectilePropultionForce;
			float rateOfFireTransferred;
			float weight;
			float aimingYaw;
			float aimingPitch;
			//static int TeamID;
			//static int personalID;
			bool canFire;
			bool AffectedGravity;
			int teamID;
			//PhysicsObject* physicsProjectile;
			Vector3 bulletDirectionVector;
			vector<Projectile*>* parentVector;

			playerTracking* player;
			GameWorld* world;


		};

	}
}
