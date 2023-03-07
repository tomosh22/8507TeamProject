#include"Projectile.h"


using namespace NCL;
using namespace CSC8503;
	

Projectile::Projectile()
{
		setGunType(pistol);
		aimingPitch = 0.0f;
		aimingYaw = 0.0f;
		//physicsProjectile = nullptr;
		bulletDirectionVector = { 0,0,0 };
		canFire = true;


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
	

