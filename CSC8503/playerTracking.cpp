#include"playerTracking.h"
#include <iostream>
#include"PhysicsObject.h"

namespace NCL::CSC8503 {

	playerTracking::playerTracking() {
		playerYawOrientation = 0.0f;
		playerPitchOrientation = 0.0f;
		playerID = 1;
		teamID = 0;
		IndividualplayerScore = 0;
		type = pistol;
		playerProjectile = new Projectile();
		paintColor = { 1,1,0,1 };
		bulletsUsed = {};
		bulletsUsedAndMoved = {};
	}


	void playerTracking::clearBulletsUsed() {
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



}


