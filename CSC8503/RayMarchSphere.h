#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class RayMarchSphere :
            public GameObject
        {
        public:
            Vector3 center;
            float radius;
            Vector3 color;
        };
    }
    
}



