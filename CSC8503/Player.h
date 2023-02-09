#pragma once

#include"GameObject.h"
namespace NCL
{
    namespace CSC8503
    {
        class Player : public GameObject
        {
        private:
            //Player Input
            int Dup;
            int Dright; 

            float DupDamp;
            float DrightDamp;
            float upDampVelocity = 0;
            float rightDampVelocity = 0;
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

            void Move();
            void Rotate();
            void Input(float dt);
            float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);


        };
    }
}