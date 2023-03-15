#pragma once
#include "Item.h"

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

			void SpawnItem();
			void SpawnHeal();
			void SpawnSpeedUp();
			void SpawnShield(); 
			void SpawnWeaponUp(); 

		protected:
			static PropSystem* _instance;
		};


	}
}