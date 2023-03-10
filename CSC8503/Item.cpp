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

	GetPhysicsObject()->SetInverseMass(0.0f);
	//GetPhysicsObject()->SetAffectedByGravity(true);
	GetPhysicsObject()->InitSphereInertia();
	SetName("Item");
	
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::PowerUpItem::PowerUpItem()
{
}

NCL::CSC8503::WeaponUpItem::WeaponUpItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(0.5f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	//GetPhysicsObject()->SetAffectedByGravity(true);
	GetPhysicsObject()->InitSphereInertia();
	SetName("WeaponUp");

	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::SpeedUpItem::SpeedUpItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(0.5f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	//GetPhysicsObject()->SetAffectedByGravity(true);
	GetPhysicsObject()->InitSphereInertia();
	SetName("Speed");

	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::ShieldItem::ShieldItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(0.5f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	//GetPhysicsObject()->SetAffectedByGravity(true);
	GetPhysicsObject()->InitSphereInertia();
	SetName("Shield");

	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::HealItem::HealItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(0.5f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	//GetPhysicsObject()->SetAffectedByGravity(true);
	GetPhysicsObject()->InitSphereInertia();
	SetName("Heal");

	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}
