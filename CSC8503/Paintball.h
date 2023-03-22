#pragma once
#include "GameObject.h"

namespace NCL
{
	namespace CSC8503
	{
		class TutorialGame;
		class Paintball : public GameObject
		{
		public:
			Paintball();
			~Paintball();
			void Update(float dt);
			void OnCollisionBegin(GameObject* object);
			bool isRed();
			void toggleRed();
		private:
			bool red;
		};
	}
}


