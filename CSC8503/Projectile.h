#pragma once

#include "Transform.h"
#include "CollisionVolume.h"
#include"GameObject.h"
#include"GameWorld.h"


namespace NCL {
	namespace CSC8503 {
		struct gun {
			float radius;
			float ProjectileSize;
			float projectileForce;
			float weight;// keep a >0 weight on instances. Otherwise applied forces will have no effect
			float rateOfFire; // time in seconds between consecuative bullets shot
			bool affectedByGravity;
		};

		static gun pistol{
			6.0f,
			0.2f,
			300,
			10.0f,
			1.0f,
			true,
		};

		static gun rocket{
			20.0f,
			0.5f,
			20.0f,
			5.0f,
			3.0f,
			true,
		};


		class NetworkObject;
		class RenderObject;
		class PhysicsObject;
		class TutorialGame;
		class CollisionInfo;
		class Projectile :public GameObject {
		public:

			/*	Projectile() {
					setGunType(pistol);
					aimingPitch = 0.0f;
					aimingYaw = 0.0f;
					physicsProjectile = nullptr;
				}*/

			Projectile();
			Projectile(gun GunToUse);
			Projectile(Vector3& position);
			//Projectile(gun GunToUse, vector<Projectile*>* parentVector, GameWorld* world) = 0;
			~Projectile() {
				//physicsProjectile;
			}

			void Update(float dt);
			void setGunType(gun wepType);


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
			//PhysicsObject* physicsProjectile;
			Vector3 bulletDirectionVector;
			vector<Projectile*>* parentVector;
			GameWorld* world;


		};

	}
}
