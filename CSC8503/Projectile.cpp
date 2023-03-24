#include"Projectile.h"
#include "NetworkedGame.h"
#include "RenderObject.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;
	

Projectile::Projectile()
{
		//setGunType(pistol);
		//aimingPitch = 0.0f;
		//aimingYaw = 0.0f;
		////physicsProjectile = nullptr;
		//bulletDirectionVector = { 0,0,0 };
		//canFire = true;
	SphereVolume* volume = new SphereVolume(0.5f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(Vector3(0,0,0));

	SetRenderObject(new RenderObject(&GetTransform(),NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(1.0f);
	GetPhysicsObject()->InitSphereInertia();
	SetLayerMask(Bullet);
	SetName("Bullet");
	AffectedGravity = true;
	SetActive(false);
	GameWorld::GetInstance()->AddGameObject(this);
}


	//Projectile::Projectile(gun GunToUse,vector<Projectile*>* parentVector,GameWorld* world) {
Projectile::Projectile(Gun GunToUse) {
		this->parentVector = parentVector;
		this->world = world;
		setGunType(GunToUse);
		aimingPitch = 0.0f;
		aimingYaw = 0.0f;
		//physicsProjectile = nullptr;
		bulletDirectionVector = { 0,0,0 };		
}

Projectile::Projectile(Vector3& position)
{
	SphereVolume* volume = new SphereVolume(0.5f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
	.SetScale(Vector3(2, 2, 2))
	.SetPosition(position);

	SetRenderObject(new RenderObject(&GetTransform(), NetworkedGame::GetInstance()->sphereMesh, nullptr, NetworkedGame::GetInstance()->basicShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(),GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(1.0f);
	GetPhysicsObject()->InitSphereInertia();

	GameWorld::GetInstance()->AddGameObject(this);
}
	


void NCL::CSC8503::Projectile::Update(float dt)
{
}

void Projectile::setGunType(Gun wepType) {
		setExplosionRadius(wepType.radius);
		setProjectileRadius(wepType.ProjectileSize);
		setProjectilePropultionForce(wepType.projectileForce);
		setWieght(wepType.weight);
		setRateOfFireTransferred(wepType.rateOfFire);
		//SetBoundingVolume(VolumeType::Sphere);
		if (wepType.affectedByGravity) {
			setAffectedByGravityTrue();
		}
		else
		{
			setAffectedByGravityFalse();
		}
	}

void NCL::CSC8503::Projectile::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetLayerMask() == Trigger)
		return;
	if (otherObject->isPaintable && otherObject != player && otherObject->GetName() != "invisible")
	{
		NetworkedGame::GetInstance()->DispatchComputeShaderForEachTriangle(otherObject, transform.GetPosition(),explosionRadius, teamID);

		player->AddScore(player->getWeaponType().addedScore);

	}
	else if (otherObject->id() == "character")
	{
		int damage = player->getWeaponType().damage;
		if (player->GetDamageUp())
			damage *= 2;
		playerTracking* enemy = static_cast<playerTracking*>(otherObject);   // safe conversion
		enemy->TakeDamage(damage); //Up To bullet
		if (enemy->GetHealth() == 0)
		{
			player->AddScore(500);
			std::cout << player->GetScore() << std::endl;
		}
	}

	//Bullet Recycle

	if (player != nullptr)
	{
		player->ReTurnBullet(this);
	}
}

void NCL::CSC8503::Projectile::OnCollisionEnd(GameObject* otherObject)
{

}
	

