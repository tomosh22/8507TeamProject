#pragma once

#include"GameObject.h"
#include"Projectile.h"
#include<vector>
#include"Vector4.h"
#include"ObjectPool.h"
#include "GameWorld.h"


namespace NCL {
	namespace CSC8503 {
		const float PLAYER_MOVE_SPEED = 10.0f;
		const float PLYAER_ITEM_SPEED_UP = 35.0f;
		const float PLAYER_SPEED_UP = 30.0f;
		const float PLAYER_JUMP_FORCE = 20.0f;

		class Team;
		class playerTracking :public GameObject {
		public:

			playerTracking();

			~playerTracking() {
				delete playerProjectile;
				bulletsUsed.clear();
				bulletsUsedAndMoved.clear();

			}

			void Update(float dt) override;

			void Move(float dt);
			void UpdateCoolDownTime(float dt);

			void Rotate();
			bool CanShoot();
			void StartShooting(Vector3 target);
			void Shooting(Projectile* bullet, Vector3 target);
			bool CanJump();
			void SpeedUp();
			void SpeedDown();
			void UpdateSpeed(float dt);
			float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

			void UpdateAimPosition(Camera* camera);

			void setplayerID(int assignedPlayerID) {
				playerID = assignedPlayerID;
			}

			void setTeamID(int assignedTeamID) {
				playerID = assignedTeamID;
			}

			int GetTeamId()
			{
				return teamID;
			}

			void SetTeamId(int team)
			{
				teamID = team;
			}

			Vector3 GetAimedTarget() const { return aimPos; }

			void resetPlayerProjectile() {
				playerProjectile = nullptr;
			}

			void setWeponType(Gun newWeponType) {
				weaponType = newWeponType; 
			}


			void addToBulletsUsed(Projectile* bulletToAdd) {
				bulletsUsed.push_back(bulletToAdd);
			}

			int getBulletVectorSize() {
				return bulletsUsed.size();
			}

			Projectile* reuseBullet();


			Gun getWeponType() {
				return weaponType;
			}

			float GetSpeed()
			{
				return moveSpeed;
			}
			void SetSpeed(float speed)
			{
				moveSpeed = speed;
			}

			int GetShield()
			{
				return shield;
			}

			void ShieldUp()
			{
				shield = 100; 
			}

			int GetHealth()
			{
				return hp;
			}

			void Heal()
			{
				hp = 100; 
			}

			void TakeSpeedUpItem()
			{
				speedUp = true;
				speedUpTimer = 300.0f; 
			}

			void WeaponUp()
			{
				weaponType = rocket;
				weaponUp = true; 
				weaponUpTimer = 300.0f; 
			}

			void Weapon(float dt);

			/*void AssignPlayerWeapon(gun weponType) {
				playerProjectile->setGunType(weponType);
			}*/

			Projectile* getPlayerProjectile() {
				return playerProjectile;
			}

			void TakeDamage(int damage);



			void FireBullet();
			void ResetBullet(Projectile* bullet);
			void ReTurnBullet(Projectile* bullet);

			void WeaponUp(Gun newGun);


			void HealthUp(Gun newGun);

			 std::string id()
			{
				return "character";
			}

		protected:
			bool canJump; 
			bool speedUp;
			bool weaponUp; 

			float playerYawOrientation;
			float playerPitchOrientation;
			
			int hp;
			int shield; 
			int playerID;
			int teamID;
			int IndividualplayerScore;
			Projectile *playerProjectile;
			Gun weaponType;
			//Vector4 paintColor;
		
			float moveSpeed;
			float sprintTimer;
			float speedUpTimer;
			float weaponUpTimer; 

			float fireOffset; //this is is offset of firing position
			Vector3 forwad;
			Vector3 right;
			Vector3 aimPos;
			//This is me 

			ObjectPool<Projectile> *bulletPool;
			float coolDownTimer;   //this is timer of firing

			vector<Projectile*> bulletsUsed;
			vector<Projectile*> bulletsUsedAndMoved;
		};




	}
}