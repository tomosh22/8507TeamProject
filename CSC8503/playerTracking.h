#pragma once

#include"GameObject.h"
#include"Projectile.h"
#include<vector>
#include"Vector4.h"
#include"ObjectPool.h"
#include "GameWorld.h"

namespace NCL {
	namespace CSC8503 {

		class playerTracking :public GameObject {
		public:

			playerTracking();

			~playerTracking() {
				delete playerProjectile;
				bulletsUsed.clear();
				bulletsUsedAndMoved.clear();

			}

			
			void Update(float dt);


			void Move(float dt);
			void Rotate();
			void Shoot(float dt);
			float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

			void setplayerID(int assignedPlayerID) {
				playerID = assignedPlayerID;
			}

			void setTeamID(int assignedTeamID) {
				playerID = assignedTeamID;
			}

			void resetPlayerProjectile() {
				playerProjectile = nullptr;
			}

			void setWeponType(gun newWeponType) {
				weaponType = newWeponType;
			}

			void setPaintColor(Vector4 newPaintColor) {
				paintColor = newPaintColor;
			}

			void addToBulletsUsed(Projectile* bulletToAdd) {
				bulletsUsed.push_back(bulletToAdd);
			}

			void clearBulletsUsed();


			int getBulletVectorSize() {
				return bulletsUsed.size();
			}

			Projectile* reuseBullet();

			Vector4 getPaintColor() {
				return paintColor;
			}

			gun getWeponType() {
				return weaponType;
			}

			/*void AssignPlayerWeapon(gun weponType) {
				playerProjectile->setGunType(weponType);
			}*/

			Projectile* getPlayerProjectile() {
				return playerProjectile;
			}

			void FireBullet();
			void ResetBullet(Projectile* bullet);
			void ReTurnBullet(Projectile* bullet);


		protected:

			float playerYawOrientation;
			float playerPitchOrientation;
			int playerID;
			int teamID;
			int IndividualplayerScore;
			Projectile *playerProjectile;
			gun weaponType;
			Vector4 paintColor;
			

			float fireOffset; //this is is offset of firing position
			Vector3 forwad;
			Vector3 right;
			//This is me 
			ObjectPool<Projectile> *bulletPool;
			float coolDownTimer;   //this is timer of firing

			vector<Projectile*> bulletsUsed;
			vector<Projectile*> bulletsUsedAndMoved;
		};




	}
}