#pragma once

#include "Item.h"
#include "RenderObject.h"
#include "GameWorld.h"

namespace NCL::CSC8503 {

	class PropSystem
	{
	public:
		PropSystem(GameWorld* gameWorld);

		Item* SpawnItem(Vector3 pos);
		Item* SpawnHeal(Vector3 pos);
		Item* SpawnSpeedUp(Vector3 pos);
		Item* SpawnShield(Vector3 pos); 
		Item* SpawnWeaponUp(Vector3 pos, PBRTextures* pbr);
	private:
		GameWorld* world = nullptr;
	};

}