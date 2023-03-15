#include "RespawnPoint.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameWorld.h"


using namespace NCL;
using namespace CSC8503;

NCL::CSC8503::RespawnPoint::RespawnPoint(Vector3 pos)
{
	SphereVolume* volume = new SphereVolume(10.0f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(pos);

	//SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(0.0f);
	//GetPhysicsObject()->SetAffectedByGravity(true);
	GetPhysicsObject()->InitSphereInertia();
	SetName("RespawnPoint");

	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

void NCL::CSC8503::RespawnPoint::AddRespawnPoint(RespawnPoint rp)
{
	respawnPoints.push_back(rp);
}

void NCL::CSC8503::RespawnPoint::FindSafeRespawn()
{

}

void NCL::CSC8503::RespawnPoint::OnCollisionBegin(GameObject* otherObject)
{
	std::cout << "In Respawn Zone!" << std::endl; 
}
