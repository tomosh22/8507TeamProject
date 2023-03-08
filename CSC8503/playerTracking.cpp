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
		

		fireOffset = 5;
		bulletPool =new ObjectPool<Projectile>();

		paintColor = { 1,1,0,1 };
		bulletsUsed = {};
		bulletsUsedAndMoved = {};
		

}

void NCL::CSC8503::playerTracking::Update(float dt)
{
	Rotate();
	Shoot();
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
	int a = Window::GetKeyboard()->KeyHeld(KeyboardKeys::A) ? 1 : 0;
	int b = Window::GetKeyboard()->KeyHeld(KeyboardKeys::D) ? 1 : 0;
	int w = Window::GetKeyboard()->KeyHeld(KeyboardKeys::W) ? 1 : 0;
	int s = Window::GetKeyboard()->KeyHeld(KeyboardKeys::S) ? 1 : 0;

	int Dup = w - s;
	int Dright = a - b;

	Vector3 moveDir;


	Matrix4 view = GameWorld::GetInstance()->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	right = Vector3(camWorld.GetColumn(0));
	forwad = Vector3::Cross(Vector3(0, 1, 0), right);
}

void NCL::CSC8503::playerTracking::Shoot()
{
	if (Window::GetMouse()->ButtonDown(MouseButtons::LEFT))
	{
		std::cout << "Shoot" << std:: endl;
		Projectile *newBullet = bulletPool->GetObject2();
		ResetBullet(newBullet);
	}
}


void playerTracking::clearBulletsUsed() 
{
		int numOfUsedBullets = bulletsUsedAndMoved.size();
		if(numOfUsedBullets > 0){
			for (int i = 0; i < 1; i++) {
				delete (bulletsUsedAndMoved[numOfUsedBullets - 1]);
			}
		}
		
}


	/*void playerTracking::updateBulletsUsed() {
		if (getBulletVectorSize() > 1) {
			for (int ix = 0; ix < getBulletVectorSize(); ix++) {
				bulletsUsed[ix]->GetTransform().Remove();
				bulletsUsedAndMoved.push_back(bulletsUsed[ix]);
				bulletsUsed.erase(bulletsUsed.begin() + ix);
				std::cout << bulletsUsedAndMoved.size() << std::endl;
			}
		}
	}*/


	

	Projectile* playerTracking::reuseBullet() {
		Projectile* reusedBullet = bulletsUsed.at(0);
		reusedBullet->GetPhysicsObject()->SetLinearVelocity(Vector3{0,0,0});
		reusedBullet->GetPhysicsObject()->ClearForces();
		
		reusedBullet->SetActive(true);
		bulletsUsed.erase(bulletsUsed.begin(),bulletsUsed.begin()+1);
		return reusedBullet;
	}

	// This is me 

void playerTracking::FireBullet() 
{		
		//Projectile newBullet = *bulletPool->GetObject2();
		//ResetBullet(newBullet);
		////Projectile* loadedBullet = useNewBullet(selectedPlayerCharacter);
		////selectedPlayerCharacter->addToBulletsUsed(loadedBullet);
		//PhysicsObject* physicsShot = newBullet.GetPhysicsObject();
		////physicsShot->SetLayerID(); // set Id so bullets cannot collied with each other and players.
		////loadedBullet->GetTransform().setDestructable();
		//physicsShot->SetLinearVelocity({ 0,0,0 });
		//physicsShot->ClearForces();

		////TODO::Force and firing direction should based on the character
		//
		//float const startingForce = newBullet.getPojectilePropultionForce();
		//Vector3 firingDirectionVector = newBullet.getBulletDirectionVector();
		//Vector3 firingDirectionVectorWithForce = firingDirectionVector * startingForce;
		//physicsShot->AddForce(firingDirectionVectorWithForce);


		//testing bullet vector removal
		/*if (selectedPlayerCharacter->getBulletVectorSize() > 10) {
			selectedPlayerCharacter->clearBulletsUsed();
		}*/
		//testing bullet vector removal
		return;
	}

//Call this function to init a new Bullet
void playerTracking::ResetBullet(Projectile* bullet)
{
	bullet->GetTransform()
		.SetScale(Vector3(weaponType.ProjectileSize, weaponType.ProjectileSize, weaponType.ProjectileSize))
		.SetPosition(transform.GetPosition()+forwad * fireOffset);
	std::cout << "Bullet pos:"<< transform.GetPosition() + forwad * fireOffset << std::endl;
	bullet->GetPhysicsObject()->SetInverseMass(weaponType.weight);
	bullet->GetPhysicsObject()->InitSphereInertia();
}
void playerTracking::ReTurnBullet(Projectile* bullet)
{

}



