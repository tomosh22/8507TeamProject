#include "PropSystem.h"

using namespace NCL;
using namespace CSC8503;


PropSystem* PropSystem::_instance = nullptr;

void NCL::CSC8503::PropSystem::SpawnItem(Vector3 pos)
{
	PowerUpItem * newItem = new PowerUpItem(pos);
}

Item* NCL::CSC8503::PropSystem::SpawnHeal(Vector3 pos)
{
	PowerUpItem* newItem = new HealItem(pos);
	return newItem;
}

Item* NCL::CSC8503::PropSystem::SpawnSpeedUp(Vector3 pos)
{
	PowerUpItem* newItem = new SpeedUpItem(pos);
	return newItem;
}

Item* NCL::CSC8503::PropSystem::SpawnShield(Vector3 pos)
{
	PowerUpItem* newItem = new ShieldItem(pos);
	return newItem;
}

Item* NCL::CSC8503::PropSystem::SpawnWeaponUp(Vector3 pos)
{
	PowerUpItem* newItem = new WeaponUpItem(pos);
	return newItem;
}
