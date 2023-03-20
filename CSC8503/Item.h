#pragma once

#include "Transform.h"
#include "CollisionVolume.h"
#include"GameObject.h"
#include"GameWorld.h"
#include "playerTracking.h"
#include "RenderObject.h"

namespace NCL {
	namespace CSC8503 {
	
		struct PBRTextures;

		class Item :public GameObject {
		public:
			void Update(float dt);
			void OnCollisionBegin(GameObject* otherObject);
			virtual void Trigger(GameObject* character) = 0;
		protected:
			Vector3 originalPos;
			float respawnTimer;
			float time = 600.0f;
			bool notSpawned = false;
		};

		class PowerUpItem :public Item {
		public:
			PowerUpItem(const Vector3 pos);
			PowerUpItem();
			~PowerUpItem() {
				//physicsProjectile;
			}
			void Trigger(GameObject* character) override {
				std::cout << "Power Up" << std::endl;
			}
		protected:

		};

		class WeaponUpItem :public PowerUpItem {
		public:
			WeaponUpItem(const Vector3 pos, PBRTextures* pbr);
			~WeaponUpItem() {
				//physicsProjectile;
			}
			void Trigger(GameObject* character) override {
				std::cout << "Weapon Up" << std::endl;
				playerTracking* c = static_cast<playerTracking*>(character);
				c->WeaponUp();
				this->GetTransform().SetPosition(Vector3(1000, 1000, 1000));
				notSpawned = true;
				respawnTimer = time;
			}
		protected:

		};

		class SpeedUpItem :public PowerUpItem {
		public:
			SpeedUpItem(const Vector3 pos);
			~SpeedUpItem() {}
			void Trigger(GameObject* character) override {
				std::cout << "Speed Up" << std::endl;
				playerTracking* c = static_cast<playerTracking*>(character);
				c->TakeSpeedUpItem();
				std::cout << c->GetSpeed() << std::endl;
				this->GetTransform().SetPosition(Vector3(1000, 1000, 1000));
				notSpawned = true;
				respawnTimer = time;
			}
		protected:

		};

		class ShieldItem :public PowerUpItem {
		public:
			ShieldItem(const Vector3 pos);
			~ShieldItem() {}
			void Trigger(GameObject* character) override {
				std::cout << "Shield" << std::endl;
				playerTracking* c  = static_cast<playerTracking*>(character);
				c->ShieldUp();
				std::cout << c->GetShield() << std::endl; 
				this->GetTransform().SetPosition(Vector3(1000, 1000, 1000));
				notSpawned = true;
				respawnTimer = time;
			}
		protected:

		};

		class HealItem :public PowerUpItem {
		public:
			HealItem(const Vector3 pos);
			~HealItem() {}
			void Trigger(GameObject* character) override {
				std::cout << "Heal" << std::endl;
				playerTracking* c = static_cast<playerTracking*>(character);
				c->Heal();
				std::cout << c->GetHealth() << std::endl;
				this->GetTransform().SetPosition(Vector3(1000, 1000, 1000));
				notSpawned = true;
				respawnTimer = time;
			}
		protected:
		};
	}
}
