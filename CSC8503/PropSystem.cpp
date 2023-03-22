#include "PropSystem.h"

using namespace NCL::CSC8503;

PropSystem::PropSystem(GameWorld* gameWorld) :
	world(gameWorld)
{

}

Item* NCL::CSC8503::PropSystem::SpawnItem(Vector3 pos)
{
	PowerUpItem* newItem = new PowerUpItem(pos);
	world->AddGameObject(newItem);
	return newItem;
}

Item* NCL::CSC8503::PropSystem::SpawnHeal(Vector3 pos)
{
	PowerUpItem* newItem = new HealItem(pos);
	world->AddGameObject(newItem);
	return newItem;
}

Item* NCL::CSC8503::PropSystem::SpawnSpeedUp(Vector3 pos)
{
	PowerUpItem* newItem = new SpeedUpItem(pos);
	world->AddGameObject(newItem);
	return newItem;
}

Item* NCL::CSC8503::PropSystem::SpawnShield(Vector3 pos)
{
	PowerUpItem* newItem = new ShieldItem(pos);
	world->AddGameObject(newItem);
	return newItem;
}

Item* NCL::CSC8503::PropSystem::SpawnWeaponUp(Vector3 pos, PBRTextures* pbr)
{
	PowerUpItem* newItem = new WeaponUpItem(pos, pbr);
	world->AddGameObject(newItem);
	return newItem;
}
