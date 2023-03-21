#pragma once
#include "GameObject.h"
#include <vector>

namespace NCL {
	namespace CSC8503 {
		class RespawnPoint : public GameObject
		{
			public:
				RespawnPoint(Vector3 pos);
				void AddRespawnPoint(RespawnPoint* rp);
				Vector3 FindSafeRespawn(int team); 
				void OnCollisionBegin(GameObject* otherObject);
				void OnCollisionEnd(GameObject* otherObject);
			protected:
				std::vector<RespawnPoint*>respawnPoints;
				bool team1Safe;
				bool team2Safe; 
		};
	}
}
