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
		respawnTimer = 200.0f; 
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
	Rotate();
	Shoot(dt);
	Move(dt);
	Jump();
	Respawning(dt);

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

	//sprint
	if (speedUp)
	{
		moveSpeed = 35;
		speedUpTimer = speedUpTimer - 10 * dt;
		if (speedUpTimer <= 0)
			speedUp = false;
	}
	else if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W) && Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT) && sprintTimer > 0) 
	{
		moveSpeed = 30;
		sprintTimer = sprintTimer - 20 * dt;
	}
	else 
	{
		moveSpeed = 10;
		sprintTimer = sprintTimer + 10 * dt;
	}



	transform.SetPosition(transform.GetPosition()+(forwad * Dup + right * Dright) * dt * moveSpeed);

}

void NCL::CSC8503::playerTracking::Shoot(float dt)
{
	//Plane plane = Plane::PlaneFromVector(Vector3(0.0f, 1000.0f, 0.0f), transform.GetDirVector());
	//aimPos = plane.GetIntersection(GameWorld::GetInstance()->GetMainCamera()->GetPosition(), GameWorld::GetInstance()->GetMainCamera()->GetForward());
	//Debug::DrawLine(transform.GetPosition(), aimPos, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	if (Window::GetMouse()->DoubleClicked(MouseButtons::LEFT)&&coolDownTimer<=0)
	{
		//Find the intersection of the plane in the player's direction and the camera ray
		Plane plane = Plane::PlaneFromVector(Vector3(0.0f, 1000.0f, 0.0f), transform.GetDirVector());
		aimPos = plane.GetIntersection(GameWorld::GetInstance()->GetMainCamera()->GetPosition(), GameWorld::GetInstance()->GetMainCamera()->GetForward());

		Projectile* newBullet = bulletPool->GetObject2();
		ResetBullet(newBullet);
		coolDownTimer = weaponType.rateOfFire;
	}
	else if (coolDownTimer > 0)
	{
		coolDownTimer -= dt;
	}
}

void NCL::CSC8503::playerTracking::Jump()
{
	RayCollision closetCollision;

	Vector3 rayPos;
	Vector3 rayDir;

	rayDir = Vector3(0, -1, 0);
	rayPos = this->GetTransform().GetPosition();

	Ray r = Ray(rayPos, rayDir);

	if (GameWorld::GetInstance()->Raycast(r, closetCollision, true, this) && closetCollision.rayDistance < 2.0f)
		canJump = true;
	else
		canJump = false;

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE) && canJump)
	{
		this->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, 20, 0));
	}
	
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
		PlayerDie();
	}
}

void NCL::CSC8503::playerTracking::PlayerDie()
{
	playerDead = true;
	speedUp = false;
	weaponUp = false;

	GetTransform().SetPosition(Vector3(2000, 2000, 2000));
	setWeponType(pistol);
}

void NCL::CSC8503::playerTracking::PlayerRespawn()
{
	playerDead = false; 
	hp = 100; 
}

void NCL::CSC8503::playerTracking::Respawning(float dt)
{
	if (playerDead)
	{
		respawnTimer = respawnTimer - 10 * dt;
		if (respawnTimer <= 0)
		{
			PlayerRespawn();
		}
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
	
	Vector3 fireDir = (aimPos - bullet->GetTransform().GetPosition()).Normalised();
	std::cout << "FireDir is :" << fireDir << std::endl;


	bullet->GetPhysicsObject()->ClearForces();
	bullet->GetPhysicsObject()->AddForce(fireDir*weaponType.projectileForce);
}
void playerTracking::ReTurnBullet(Projectile* bullet)
{
	bullet->SetActive(false);
	bullet->GetPhysicsObject()->ClearForces();
	bulletPool->ReturnObject(bullet);
}



