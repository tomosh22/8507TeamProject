#pragma once
#include "GameObject.h"

namespace NCL
{
	namespace CSC8503
	{
		class Paintball : public GameObject
		{
		public:
			Paintball();
			~Paintball();
			void Update(float dt);
			void OnCollisionBegin(GameObject* object);
			bool isRed();
		private:
			bool red;
			GameObject* paintball; 
		};
	}
}


