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

	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));
	GetPhysicsObject()->SetInverseMass(0.0f);
	
	SetName("RespawnPoint");
	SetLayerMask(Trigger);
	SetActive(true);
	GameWorld::GetInstance()->AddGameObject(this);
}

void NCL::CSC8503::RespawnPoint::AddRespawnPoint(RespawnPoint* rp)
{
	respawnPoints.push_back(rp);
}

Vector3 NCL::CSC8503::RespawnPoint::FindSafeRespawn(int team)
{
	Vector3 position; 
	for (int i = 0; i < respawnPoints.size(); i++)
	{
		if (team == 1)
		{
			if (respawnPoints[i]->team1Safe == true)
			{
				position = respawnPoints[i]->GetTransform().GetPosition();
			}
		}
		else if (respawnPoints[i]->team2Safe == true)
		{
			position = respawnPoints[i]->GetTransform().GetPosition();
		}
	}
	return position; 
}

void NCL::CSC8503::RespawnPoint::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "character")
		team2Safe = false;

	if (otherObject->GetName() == "character2")
		team1Safe = false;
	
}

void NCL::CSC8503::RespawnPoint::OnCollisionEnd(GameObject* otherObject)
{
	if (otherObject->GetName() == "character")
		team2Safe = true;

	if (otherObject->GetName() == "character2")
		team1Safe = true;
}
