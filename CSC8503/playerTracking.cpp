#include"playerTracking.h"
#include <iostream>
#include"PhysicsObject.h"
#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

playerTracking::playerTracking() 
{
		playerYawOrientation = 0.0f;
		playerPitchOrientation = 0.0f;
		playerID = 1;
		teamID = 0;
		IndividualplayerScore = 0;
		weaponType = pistol;
		moveSpeed = 10;
		sprintTimer = 5.0f; 
		speedUp = false; 
		weaponUp = false; 
		hp = 100;
		shield = 0; 

		fireOffset = 10;
		bulletPool = new ObjectPool<Projectile>();
		coolDownTimer = 0;
		bulletsUsed = {};
		bulletsUsedAndMoved = {};

}

void NCL::CSC8503::playerTracking::Update(float dt)
{
	UpdateSpeed(dt);
	UpdateCoolDownTime(dt);
	//test damage
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::L))
	{
		TakeDamage(50);
	}
}

void NCL::CSC8503::playerTracking::Rotate()
{
	float yaw = GameWorld::GetInstance()->GetMainCamera()->GetYaw();

	Quaternion qua = Quaternion::EulerAnglesToQuaternion(0, yaw, 0);
	transform.SetOrientation(qua);

}

void playerTracking::SpeedUp() {
	if (sprintTimer > 0.0f) {
		moveSpeed = PLAYER_SPEED_UP;
	}
}

void playerTracking::SpeedDown() {
	moveSpeed = PLAYER_MOVE_SPEED;
}

void playerTracking::UpdateSpeed(float dt) {
	//sprint
	if (speedUp)
	{
		moveSpeed = PLYAER_ITEM_SPEED_UP;
		speedUpTimer = speedUpTimer - 10 * dt;
		if (speedUpTimer <= 0)
			speedUp = false;
	}
	else if (MINIMAL_NUMBER > moveSpeed - PLYAER_ITEM_SPEED_UP || MINIMAL_NUMBER < moveSpeed - PLYAER_ITEM_SPEED_UP)
	{
		sprintTimer = sprintTimer - 20 * dt;
	}
	else
	{
		sprintTimer = sprintTimer + 10 * dt;
	}
}

void playerTracking::UpdateAimPosition(Camera* camera) {
	if (!camera) { return; }
	Plane plane = Plane::PlaneFromVector(Vector3(0.0f, 100.0f, 0.0f), transform.GetDirVector());
	aimPos = plane.GetIntersection(camera->GetPosition(), camera->GetForward());
}

bool playerTracking::CanShoot() {
	return coolDownTimer <= 0.0f;
}

void playerTracking::UpdateCoolDownTime(float dt) {
	coolDownTimer -= dt;
}

//Targets are passed in from outside and can be reused for network players
void NCL::CSC8503::playerTracking::StartShooting(Vector3 target)
{
	Projectile* newBullet = bulletPool->GetObject2();
	ResetBullet(newBullet);
	coolDownTimer = weaponType.rateOfFire;
	Shooting(newBullet, target);
}

//Call this function to init a new Bullet
void playerTracking::ResetBullet(Projectile* bullet)
{

	CapsuleVolume* volume = new CapsuleVolume(weaponType.ProjectileSize * 2.0f, weaponType.ProjectileSize);
	bullet->SetBoundingVolume((CollisionVolume*)volume);

	bullet->GetTransform().SetScale(Vector3(weaponType.ProjectileSize, weaponType.ProjectileSize*2.0, weaponType.ProjectileSize)).SetPosition(transform.GetPosition() + transform.GetDirVector().Normalised() * 2.5f + Vector3(0.0f, 1.8f, 0.0f));

	bullet->SetPhysicsObject(new PhysicsObject(&bullet->GetTransform(), bullet->GetBoundingVolume()));

	bullet->GetPhysicsObject()->SetInverseMass(weaponType.weight);
	bullet->GetPhysicsObject()->InitSphereInertia();
	bullet->SetTeamID(teamID);
	bullet->SetPlayer(this);
	bullet->setExplosionRadius(weaponType.radius);
	bullet->SetActive(true);
	bullet->GetPhysicsObject()->SetLinearVelocity({ 0,0,0 });
	bullet->GetPhysicsObject()->ClearForces();
}

void playerTracking::Shooting(Projectile* bullet, Vector3 target) {
	Vector3 fireDir = (target - bullet->GetTransform().GetPosition());
	fireDir.Normalise();
	bullet->GetPhysicsObject()->AddForce(fireDir * weaponType.projectileForce);
}

bool NCL::CSC8503::playerTracking::CanJump(){
	RayCollision closetCollision;

	Vector3 rayPos;
	Vector3 rayDir;

	Vector3 pos = this->GetTransform().GetPosition();
	rayDir = Vector3(0, -1, 0);
	rayPos = pos;

	Ray r = Ray(rayPos, rayDir);

	if (GameWorld::GetInstance()->Raycast(r, closetCollision, true, this) && closetCollision.rayDistance < 0.8f)
		return true;
	else
		return false;

}


void NCL::CSC8503::playerTracking::Weapon(float dt)
{
	if (weaponUp)
	{
		weaponUpTimer = weaponUpTimer - 10 * dt;
		if (weaponUpTimer <= 0)
		{
			weaponUp = false;
			setWeponType(pistol);
		}
	}
}

void NCL::CSC8503::playerTracking::TakeDamage(int damage)
{
	if (shield <= 0)
	{
		hp = hp - damage > 0 ? hp - damage : 0;
	}
	else
		shield = shield - damage > 0 ? shield - damage : 0;
	if (hp <= 0)
	{
		//Die and ReSpawn
		std::cout << "Player Dead" << std::endl;
	}
}

void playerTracking::ReTurnBullet(Projectile* bullet)
{
	bullet->SetActive(false);
	bullet->GetPhysicsObject()->ClearForces();
	bulletPool->ReturnObject(bullet);
}

void playerTracking::WeaponUp(Gun newGun)
{
	weaponType = newGun;
}



void playerTracking::HealthUp(Gun newGun)
{
	
}

