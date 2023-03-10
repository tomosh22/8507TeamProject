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
			virtual void Trigger() =0;
		protected:

		};

		class PowerUpItem :public Item {
		public:
			PowerUpItem(const Vector3 pos);
			~PowerUpItem() {
				//physicsProjectile;
			}
			void Trigger() override {
				std::cout << "Power Up" << std::endl;
				//Recyle
			}
		protected:

		};


	}
}
