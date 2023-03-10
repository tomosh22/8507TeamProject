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

		fireOffset = 10;
		bulletPool =new ObjectPool<Projectile>();
		coolDownTimer = 0;
		bulletsUsed = {};
		bulletsUsedAndMoved = {};

}

void NCL::CSC8503::playerTracking::Update(float dt)
{
	Rotate();
	Shoot(dt);
	Move(dt);
}

void NCL::CSC8503::playerTracking::Rotate()
{
	float yaw = GameWorld::GetInstance()->GetMainCamera()->GetYaw();

	Quaternion qua = Quaternion::EulerAnglesToQuaternion(0, yaw, 0);
	transform.SetOrientation(qua);

}

void NCL::CSC8503::playerTracking::Move(float dt)
{
	int a = Window::GetKeyboard()->KeyDown(KeyboardKeys::A) ? 1 : 0;
	int b = Window::GetKeyboard()->KeyDown(KeyboardKeys::D) ? 1 : 0;
	int w = Window::GetKeyboard()->KeyDown(KeyboardKeys::W) ? 1 : 0;
	int s = Window::GetKeyboard()->KeyDown(KeyboardKeys::S) ? 1 : 0;

	int Dup = w - s;
	int Dright = b - a;

	Vector3 moveDir;

	//std::cout << "up:" << Dup << " Dright:" << Dright << std::endl;

	Matrix4 view = GameWorld::GetInstance()->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	right = Vector3(camWorld.GetColumn(0));
	forwad = Vector3::Cross(Vector3(0, 1, 0), right);


	transform.SetPosition(transform.GetPosition()+(forwad * Dup + right * Dright)*dt*moveSpeed);

}

void NCL::CSC8503::playerTracking::Shoot(float dt)
{
	if (Window::GetMouse()->DoubleClicked(MouseButtons::LEFT)&&coolDownTimer<=0)
	{
		RayCollision closestCollision;
		Ray r = Ray(GameWorld::GetInstance()->GetMainCamera()->GetPosition(), GameWorld::GetInstance()->GetMainCamera()->GetForward());

		if (GameWorld::GetInstance()->Raycast(r, closestCollision, true)) {
			aimDir = closestCollision.collidedAt;
		}

		Projectile* newBullet = bulletPool->GetObject2();
		ResetBullet(newBullet);
		coolDownTimer = weaponType.rateOfFire;
	}
	else if (coolDownTimer > 0)
	{
		coolDownTimer -= dt;
	}
}


void NCL::CSC8503::playerTracking::TakeDamage(int damage)
{
	hp = hp - damage > 0 ? hp - damage : 0;
	if (hp <= 0)
	{
		//Die and ReSpawn
	}
}

//Call this function to init a new Bullet
void playerTracking::ResetBullet(Projectile* bullet)
{
	bullet->GetTransform()
		.SetScale(Vector3(weaponType.ProjectileSize, weaponType.ProjectileSize, weaponType.ProjectileSize))
		.SetPosition(transform.GetPosition() + forwad * fireOffset + Vector3(0, 1.8, 0));
	//std::cout << "Bullet pos:"<< transform.GetPosition() + forwad * fireOffset << std::endl;

	bullet->GetPhysicsObject()->SetInverseMass(weaponType.weight);
	bullet->GetPhysicsObject()->InitSphereInertia();
	bullet->SetTeamID(teamID);
	bullet->SetPlayer(this);
	bullet->setExplosionRadius(weaponType.radius);
	bullet->SetActive(true);
	
	Vector3 fireDir = (aimDir - bullet->GetTransform().GetPosition()).Normalised();
	std::cout << "FireDir is :" << fireDir << std::endl;


	bullet->GetPhysicsObject()->ClearForces();
	bullet->GetPhysicsObject()->AddForce(fireDir*weaponType.projectileForce);
	//Debug::DrawLine(transform.GetPosition() + forwad * fireOffset + Vector3(0, 1.8, 0), fireDir * 30 + transform.GetPosition() + forwad * fireOffset + Vector3(0, 1.8, 0), Vector4(1, 0, 1, 1), 100.f);
}
void playerTracking::ReTurnBullet(Projectile* bullet)
{
	bullet->SetActive(false);
	bullet->GetPhysicsObject()->ClearForces();
	bulletPool->ReturnObject(bullet);
}



