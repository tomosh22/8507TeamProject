#include "PropSystem.h"

using namespace NCL;
using namespace CSC8503;


PropSystem* PropSystem::_instance = nullptr;

void NCL::CSC8503::PropSystem::SpawnItem()
{
	PowerUpItem * newItem = new PowerUpItem(Vector3(6, 6, 6));
}
