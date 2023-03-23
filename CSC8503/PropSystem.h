#pragma once
#include "Item.h"
#include "RenderObject.h"

namespace NCL {
	namespace CSC8503 {
		class PropSystem
		{
		public:
		
			static PropSystem* GetInstance()
			{
				if (_instance == nullptr)
				{
					_instance = new PropSystem();
				}
				return _instance;
			}

			void SpawnItem(Vector3 pos);
			Item* SpawnHeal(Vector3 pos);
			Item* SpawnSpeedUp(Vector3 pos);
			Item* SpawnShield(Vector3 pos); 
			Item* SpawnDamageUp(Vector3 pos);
			Item* SpawnWeaponUp(Vector3 pos);

		protected:
			static PropSystem* _instance;
		};


	}
}