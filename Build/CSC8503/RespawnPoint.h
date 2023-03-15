#pragma once
#include "GameObject.h"
#include <vector>

namespace NCL {
	namespace CSC8503 {
		class RespawnPoint : public GameObject
		{
			public:
				RespawnPoint(Vector3 pos);
				void AddRespawnPoint(RespawnPoint rp);
				void FindSafeRespawn(); 
				void OnCollisionBegin(GameObject* otherObject);
			protected:
				std::vector<RespawnPoint>respawnPoints;
		};
	}
}
