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

		protected:
			static PropSystem* _instance;
		};


	}
}