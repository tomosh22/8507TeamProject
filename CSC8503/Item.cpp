#include"Item.h"
#include "NetworkedGame.h"
#include "RenderObject.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

void NCL::CSC8503::Item::Update(float dt)
{
	if (notSpawned)
		respawnTimer = respawnTimer - 10 * dt;
	if (respawnTimer <= 0)
	{
		notSpawned = false;
		this->GetTransform().SetPosition(originalPos);
	}
}

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

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->powerUpMesh, NetworkedGame::GetInstance()->powerUpTex, NetworkedGame::GetInstance()->basicShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	
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
	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(1.0, 1.0, 1.0))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->powerUpMesh, NetworkedGame::GetInstance()->powerUpTex, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(1, 0, 0, 1)); 
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	
	GetPhysicsObject()->InitSphereInertia();
	SetName("WeaponUp");
	originalPos = pos;
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::SpeedUpItem::SpeedUpItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(1.0, 1.0, 1.0))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->powerUpMesh, NetworkedGame::GetInstance()->powerUpTex, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	
	GetPhysicsObject()->InitSphereInertia();
	SetName("Speed");
	originalPos = pos;
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::ShieldItem::ShieldItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(1.0, 1.0, 1.0))
		.SetPosition(pos);
	std::cout << "shield" << NetworkedGame::GetInstance()->powerUpTex << '\n';
	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->powerUpMesh, NetworkedGame::GetInstance()->powerUpTex, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	GetRenderObject()->name = "item";
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	
	GetPhysicsObject()->InitSphereInertia();
	SetName("Shield");
	originalPos = pos;
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::HealItem::HealItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(1.0, 1.0, 1.0))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->healMesh, NetworkedGame::GetInstance()->powerUpTex, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	
	GetPhysicsObject()->InitSphereInertia();
	SetName("Heal");
	originalPos = pos;
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

NCL::CSC8503::DamageUpItem::DamageUpItem(const Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(1.0, 1.0, 1.0))
		.SetPosition(pos);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);

	GetPhysicsObject()->InitSphereInertia();
	SetName("DamageUp");
	originalPos = pos;
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}
