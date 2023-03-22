#include "RespawnPoint.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameWorld.h"
#include "NetworkedGame.h"


using namespace NCL;
using namespace CSC8503;

NCL::CSC8503::RespawnPoint::RespawnPoint(Vector3 pos)
{
	Vector3 size = { 20.0f, 1.0f, 20.0f };
	AABBVolume* volume = new AABBVolume(size);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(size)
		.SetPosition(pos);

	//SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->cubeMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));
	GetPhysicsObject()->SetInverseMass(0.0f);
	
	SetName("RespawnPoint");
	SetLayerMask(Trigger);
	team1Safe = true;
	team2Safe = true;
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);

	
}

void NCL::CSC8503::RespawnPoint::AddRespawnPoint(RespawnPoint* rp)
{
	respawnPoints.push_back(rp);
}

Vector3 NCL::CSC8503::RespawnPoint::FindSafeRespawn(int team)
{
	for (int i = 0; i < respawnPoints.size(); i++)
	{
		if (team == 1)
		{
			if (respawnPoints[i]->team1Safe)
			{
				return respawnPoints[i]->GetTransform().GetPosition();
			}
		}
		else if (respawnPoints[i]->team2Safe)
		{
			return respawnPoints[i]->GetTransform().GetPosition();
		}
	}

	return Vector3(0, 100, 0);
}

void NCL::CSC8503::RespawnPoint::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "character")
		team2Safe = false;

	if (otherObject->GetName() == "character2")
		team1Safe = false;

	if (otherObject->GetName() == "character")
		std::cout << "Colliding with player" << std::endl;
}

void NCL::CSC8503::RespawnPoint::OnCollisionEnd(GameObject* otherObject)
{
	if (otherObject->GetName() == "character")
		team2Safe = true;

	if (otherObject->GetName() == "character2")
		team1Safe = true;
}
