#pragma once

#include "Transform.h"
#include "CollisionVolume.h"
#include"GameObject.h"
#include"GameWorld.h"

namespace NCL {
	namespace CSC8503 {
	
		class Item :public GameObject {
		public:
			void OnCollisionBegin(GameObject* otherObject);
			virtual void Trigger(GameObject* character) =0;
		protected:

		};

		class PowerUpItem :public Item {
		public:
			PowerUpItem(const Vector3 pos);
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
			WeaponUpItem(const Vector3 pos);
			~WeaponUpItem() {
				//physicsProjectile;
			}
			void Trigger(GameObject* character) override {
				std::cout << "Weapon Up" << std::endl;
			}
		protected:

		};

		class SpeedUpItem :public PowerUpItem {
		public:
			SpeedUpItem(const Vector3 pos);
			~SpeedUpItem() {}
			void Trigger(GameObject* character) override {
				std::cout << "Speed Up" << std::endl;
			}
		protected:

		};

		class ShieldItem :public PowerUpItem {
		public:
			ShieldItem(const Vector3 pos);
			~ShieldItem() {}
			void Trigger(GameObject* character) override {
				std::cout << "Shield" << std::endl;
			}
		protected:

		};

		class HealItem :public PowerUpItem {
		public:
			HealItem(const Vector3 pos);
			~HealItem() {}
			void Trigger(GameObject* character) override {
				std::cout << "Shield" << std::endl;
			}
		protected:

		};
	}
}
