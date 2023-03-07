#include"playerTracking.h"
#include <iostream>
#include"PhysicsObject.h"
#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;



playerTracking::playerTracking() {
		playerYawOrientation = 0.0f;
		playerPitchOrientation = 0.0f;
		playerID = 1;
		teamID = 0;
		IndividualplayerScore = 0;
		type = pistol;
		
		bulletPool =new ObjectPool<Projectile>(10);

		playerProjectile = new Projectile();
		paintColor = { 1,1,0,1 };
		bulletsUsed = {};
		bulletsUsedAndMoved = {};


	}

void NCL::CSC8503::playerTracking::Update(float dt)
{
	Rotate();
}
void NCL::CSC8503::playerTracking::Rotate()
{

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
		Projectile newBullet = *bulletPool->GetObject2();
		ResetBullet(newBullet);
		//Projectile* loadedBullet = useNewBullet(selectedPlayerCharacter);
		//selectedPlayerCharacter->addToBulletsUsed(loadedBullet);
		PhysicsObject* physicsShot = newBullet.GetPhysicsObject();
		//physicsShot->SetLayerID(); // set Id so bullets cannot collied with each other and players.
		//loadedBullet->GetTransform().setDestructable();
		physicsShot->SetLinearVelocity({ 0,0,0 });
		physicsShot->ClearForces();

		//TODO::Force and firing direction should based on the character
		
		float const startingForce = newBullet.getPojectilePropultionForce();
		Vector3 firingDirectionVector = newBullet.getBulletDirectionVector();
		Vector3 firingDirectionVectorWithForce = firingDirectionVector * startingForce;
		physicsShot->AddForce(firingDirectionVectorWithForce);


		//testing bullet vector removal
		/*if (selectedPlayerCharacter->getBulletVectorSize() > 10) {
			selectedPlayerCharacter->clearBulletsUsed();
		}*/
		//testing bullet vector removal
		return;
	}
void playerTracking::ResetBullet(Projectile bullet)
{

}
void playerTracking::ReTurnBullet(Projectile bullet)
{

}



