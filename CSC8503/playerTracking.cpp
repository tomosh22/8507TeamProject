#include"playerTracking.h"
#include <iostream>
#include"PhysicsObject.h"
#include "TutorialGame.h"
#include "RespawnPoint.h"

using namespace NCL;
using namespace CSC8503;

playerTracking::playerTracking() 
{
		playerYawOrientation = 0.0f;
		playerPitchOrientation = 0.0f;
		playerID = 1;
		teamID = 0;
		IndividualplayerScore = 0;
		
		moveSpeed = PLAYER_MOVE_SPEED;
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
		//weapon
		weaponInUse = pistol;
		weaponPool.push_back(pistol);
		weaponPool.push_back(rocket);

		animationMap["Idle"] = new  NCL::MeshAnimation("Idle.anm");
		animationMap["MoveF"] = new  NCL::MeshAnimation("RunForward.anm");
		animationMap["MoveB"] = new  NCL::MeshAnimation("RunBackward.anm");
		animationMap["MoveL"] = new  NCL::MeshAnimation("RunLeft.anm");
		animationMap["MoveR"] = new  NCL::MeshAnimation("RunRight.anm");
		currentAniamtion = animationMap["Idle"];

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
	GetRenderObject()->anim = currentAniamtion;
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

void playerTracking::SpeedDown() 
{
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
	else if (MINIMAL_NUMBER > moveSpeed - PLAYER_SPEED_UP && MINIMAL_NUMBER < moveSpeed - PLAYER_SPEED_UP)
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
	newBullet->SetName("Bulllet");
	ResetBullet(newBullet);
	coolDownTimer = weaponInUse.rateOfFire;
	Shooting(newBullet, target);
}

//Call this function to init a new Bullet
void playerTracking::ResetBullet(Projectile* bullet)
{
	CapsuleVolume* volume = new CapsuleVolume(weaponInUse.ProjectileSize * 2.0f, weaponInUse.ProjectileSize);
	bullet->SetBoundingVolume((CollisionVolume*)volume);

	bullet->GetTransform().SetScale(Vector3(weaponInUse.ProjectileSize, weaponInUse.ProjectileSize*2.0, weaponInUse.ProjectileSize)).SetPosition(transform.GetPosition() + Vector3(0.0f, 2.0f, 0.0f));

	bullet->SetPhysicsObject(new PhysicsObject(&bullet->GetTransform(), bullet->GetBoundingVolume()));

	bullet->GetPhysicsObject()->SetInverseMass(weaponInUse.weight);
	bullet->GetPhysicsObject()->InitSphereInertia();
	bullet->SetTeamID(teamID);
	bullet->SetPlayer(this);
	bullet->setExplosionRadius(weaponInUse.radius);
	bullet->SetActive(true);
	bullet->GetPhysicsObject()->SetLinearVelocity({ 0,0,0 });
	bullet->GetPhysicsObject()->ClearForces();
}

void playerTracking::Shooting(Projectile* bullet, Vector3 target) {
	Vector3 fireDir = (target - bullet->GetTransform().GetPosition());
	fireDir.Normalise();
	bullet->GetPhysicsObject()->AddForce(fireDir * weaponInUse.projectileForce);
	std::cout << "Shooting, teamId: " << teamID << ", position: " << bullet->GetTransform().GetPosition() << ", target : " << target << std::endl;
}

bool NCL::CSC8503::playerTracking::CanJump(GameObject* floor){
	RayCollision closetCollision;
	Ray r = Ray(transform.GetPosition(), Vector3(0, -1, 0));

	if (CollisionDetection::RayIntersection(r, *floor, closetCollision) && closetCollision.rayDistance < 1.0f) {
		return true;
	}
	else {
		return false;
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

void NCL::CSC8503::playerTracking::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "ladder")
		onLadder = true;
}

void NCL::CSC8503::playerTracking::OnCollisionEnd(GameObject* otherObject)
{
	if (otherObject->GetName() == "ladder")
		onLadder = false;
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
	Vector3 position;
	playerDead = false; 
	hp = 100; 
	position = respawn->FindSafeRespawn(teamID);
	respawnTimer = 50.0f;
	GetTransform().SetPosition(position);
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

void playerTracking::ReTurnBullet(Projectile* bullet)
{
	bullet->SetActive(false);
	bullet->GetPhysicsObject()->ClearForces();
	bulletPool->ReturnObject(bullet);
}

void playerTracking::WeaponUp(Gun newGun)
{
	weaponInUse = newGun;
}



void playerTracking::HealthUp(Gun newGun)
{
	
}

void playerTracking::WriteActionMessage(int actionTp, int param) {
	if (!online) { return; }
	switch (actionTp) {
	case PLAYER_ACTION_SWITCH_WEAPON:
	{
		auto packet = new ActionPacket(networkID, PLAYER_ACTION_SWITCH_WEAPON);
		sendMessagePool.push_back(packet);
		break;
	}
	case PLAYER_ACTION_SHOOT:
	{
		auto state = NetworkState();
		state.position = transform.GetPosition();
		state.orientation = transform.GetOrientation();
		auto packet = new ActionPacket(networkID, PLAYER_ACTION_SHOOT, aimPos, state);
		sendMessagePool.push_back(packet);
		break;
	}
	case Select_Player_Mode:
	{
		auto packet = new SelectModePacket(networkID, param);
		sendMessagePool.push_back(packet);
		break;
	}
	default:
		std::cout << "Error player action" << std::endl;
	}
}

void playerTracking::PrintPlayerInfo() {
	Debug::Print("Health: " + std::to_string(hp), Vector2(5, 90), Debug::RED);
	Debug::Print("Shield: " + std::to_string(shield), Vector2(5, 95), Debug::CYAN);

	string text;
	if (weaponInUse.type == GUN_TYPE_PISTOL) {
		text = "WEAPON: PISTOL";
	} 
	else if (weaponInUse.type == GUN_TYPE_ROCKET) {
		text = "WEAPON: ROCKET";
	}
	Debug::Print(text, Vector2(70, 95), Debug::WHITE);
}

void playerTracking::UpdateAction(ActionPacket packet) {
	if (packet.buttonstates[PLAYER_ACTION_SWITCH_WEAPON]) {
		SwitchWeapon();
	}
	if (packet.buttonstates[PLAYER_ACTION_SHOOT]) {
		transform.SetPosition(packet.state.position);
		transform.SetOrientation(packet.state.orientation);
		StartShooting(packet.param);
	}
}

void NCL::CSC8503::playerTracking::TransferAnimation(std::string animationName)
{

	if (currentAniamtion == animationMap[animationName])
	{
		return;
	}
	currentAniamtion = animationMap[animationName];
}

void NCL::CSC8503::playerTracking::AddScore(int score)
{
	IndividualplayerScore += score; 
}
