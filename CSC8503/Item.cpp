#include"Item.h"
#include "NetworkedGame.h"
#include "RenderObject.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

void NCL::CSC8503::Item::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "character")
	{
		Trigger(otherObject);
	}
}




NCL::CSC8503::PowerUpItem::PowerUpItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(0.5f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(1.0f);
	GetPhysicsObject()->SetAffectedByGravity(true);
	GetPhysicsObject()->InitSphereInertia();
	SetName("Item");
	
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}
