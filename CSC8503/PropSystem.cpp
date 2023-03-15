#include "PropSystem.h"

using namespace NCL;
using namespace CSC8503;


PropSystem* PropSystem::_instance = nullptr;

void NCL::CSC8503::PropSystem::SpawnItem()
{
	PowerUpItem * newItem = new PowerUpItem(Vector3(6, 3, 6));
}

void NCL::CSC8503::PropSystem::SpawnHeal()
{
	PowerUpItem* newItem = new HealItem(Vector3(9, 3, 6));
}

void NCL::CSC8503::PropSystem::SpawnSpeedUp()
{
	PowerUpItem* newItem = new SpeedUpItem(Vector3(12, 3, 6));
}

void NCL::CSC8503::PropSystem::SpawnShield()
{
	PowerUpItem* newItem = new ShieldItem(Vector3(15, 3, 6));
}

void NCL::CSC8503::PropSystem::SpawnWeaponUp()
{
	PowerUpItem* newItem = new WeaponUpItem(Vector3(18, 3, 6));
}
