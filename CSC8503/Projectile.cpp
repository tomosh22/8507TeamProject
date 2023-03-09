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
	SetName("Bullet");
	AffectedGravity = true;
	SetActive(false);
	GameWorld::GetInstance()->AddGameObject(this);
}


	//Projectile::Projectile(gun GunToUse,vector<Projectile*>* parentVector,GameWorld* world) {
Projectile::Projectile(gun GunToUse) {
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

void Projectile::setGunType(gun wepType) {
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
	if (otherObject->isPaintable)
	{
		//Bullet Recycle
		if (player!=nullptr)
		{
			player->ReTurnBullet(this);
		}
	}
}

void NCL::CSC8503::Projectile::OnCollisionEnd(GameObject* otherObject)
{

}
	

