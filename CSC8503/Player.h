#pragma once

#include"GameObject.h"
namespace NCL
{
    namespace CSC8503
    {
        class Player : public GameObject
        {
        public:
            Player();
            ~Player();

            void Update(float dt);

            inline void OnCollisionBegin(GameObject* otherObject) 
            {
                if (otherObject->GetName() == "Bonus")
                    {
                        otherObject->SetActive(false);
                    }
            }
        };
    }
}