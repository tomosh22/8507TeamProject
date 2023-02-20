#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
//#include "C:\Users\c2065963\source\repos\Advanced compter physics\Build\CSC8503\AgentMovement.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include<cmath>
//#include"C:\Users\c2065963\source\repos\Advanced compter physics\Build\CSC8503\Print.h"
//#include "C:\Users\c2065963\source\repos\Advanced compter physics\Build\CSC8503\TimeKeeper.h"
#include "PushdownState.h"


#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
//#include "C:\Users\c2065963\source\repos\Advanced compter physics\Build\CSC8503\BehviourAll.h"
#include "playerTracking.h"
#include"Projectile.h"


#include<iostream>
using namespace std;
using namespace NCL;
using namespace CSC8503;
TutorialGame::TutorialGame()	{
	//amovement = new AgentMovement();
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;
	testStateObject = nullptr;
	
	objectpool = new ObjectPool<Projectile>();

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");



	InitCamera();
	//InitWorld();

	AddFloorToWorld({ 0, 0, 0 });
	AddPlayerToWorld({ 0, 1, 0 });
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
	delete amovement;

	delete objectpool;
}




//void TutorialGame::updateBehaviourTree(BehaviourSequence* root1) {
//
//}

 //bring in the behaviour tree

//void UseBehaviourTree(GameObject* importAI,GameObject* importPlayer,float ndt) {
//	float behaviourTimer;
//	float distanceToTarget;
//
//	AgentMovement* AIAgentMovementObject = importAI->GetAgentObject();
//	BehaviourAction* checkSafety = new BehaviourAction("check safety", [&](float dt, BehaviourState state)->BehaviourState {
//		if (state == Initialise) {
//
//			state = Ongoing;
//
//		}
//		else if (state == Ongoing) {
//			
//			if (importAI->GetAgentObject()->checkWithinRadius(importAI, Vector3(0, -20, 0), 15.0f) || (importAI->GetAgentObject()->checkPlayerDistance(importAI, importPlayer))<=7.0f) {
//				std::cout << "success" << std::endl;
//				return Success;
//			}
//			return Success;
//
//		}
//		return state; //will be ’ongoing ’ until success
//		}
//	);
//
//	BehaviourAction* goToPlayer = new BehaviourAction("Go To Player",
//		[&](float dt, BehaviourState state)->BehaviourState {
//			if (state == Initialise) {
//				std::cout << "Going to the player\n";
//				state = Ongoing;
//
//			}
//			else if (state == Ongoing) {
//				importAI->GetAgentObject()->headToPlayer(importAI,importPlayer);
//				std::cout << "moved towards player" << std::endl;
//				return Success;
//			}
//			return state; //will be ’ongoing ’ until success
//		}
//	);
//
//	BehaviourAction* openDoor = new BehaviourAction("Open Door",
//		[&](float dt, BehaviourState state)->BehaviourState {
//			if (state == Initialise) {
//				std::cout << "Opening Door!\n";
//				return Success;
//
//			}
//			return state;
//		}
//	);
//	BehaviourAction* HeadToCentre = new BehaviourAction(
//		"Head To Centre",
//		[&](float dt, BehaviourState state)->BehaviourState {
//			if (state == Initialise) {
//				std::cout << "Heading to the centre !\n";
//				return Ongoing;
//
//			}
//			else if (state == Ongoing) {
//				if (!(importAI->GetAgentObject()->checkWithinRadius(importAI, Vector3(0, -20, 0), 10.0f)) && !((importAI->GetAgentObject()->checkPlayerDistance(importAI, importPlayer)) <= 7.0f)) {
//					importAI->GetAgentObject()->headToPoint(importAI, Vector3(0, -17, 0));
//				}
//				std::cout << "reached here !!!" << std::endl;
//				return Success;
//
//				
//			}
//			return state;
//		}
//	);
//	BehaviourAction* lookForItems = new BehaviourAction(
//		"Look For Items",
//		[&](float dt, BehaviourState state)->BehaviourState {
//			if (state == Initialise) {
//				std::cout << "Looking for items!\n";
//				return Ongoing;
//
//			}
//			else if (state == Ongoing) {
//				bool found = rand() % 2;
//				if (found) {
//					std::cout << "I found some items!\n";
//					return Success;
//
//				}
//				std::cout << "No items in here ...\n";
//				return Failure;
//
//			}
//			return state;
//		}
//	);
	//BehaviourSequence* sequence =
	//	new BehaviourSequence("Room Sequence");
	//sequence->AddChild(checkSafety);
	//sequence->AddChild(goToPlayer);
	///*sequence->AddChild(HeadToCentre);*/
	////sequence->AddChild(openDoor);

	//BehaviourSelector* selection =
	//	new BehaviourSelector("Loot Selection");
	///*selection->AddChild(checkSafety);
	//selection->AddChild(goToPlayer);*/
	//selection->AddChild(HeadToCentre);
	////selection->AddChild(lookForItems);

	//BehaviourSequence* rootSequence =
	//	new BehaviourSequence("Root Sequence");
	//rootSequence->AddChild(sequence);
	//rootSequence->AddChild(selection);
	//rootSequence->Reset();
	//behaviourTimer = 0.0f;
	//BehaviourState state = Ongoing;

	//while (state == Ongoing) {
	//	state = rootSequence->Execute(ndt); //fake dt

	//}
	//if (state == Success) {
	//	std::cout << "What a successful adventure !\n";

	//}
	//else if (state == Failure) {
	//	std::cout << "What a waste of time!\n";

	//}

 //}
	//rootSequence->AddChild(selection);
//	for (int i = 0; i < 5; ++i) {
//		rootSequence->Reset();
//		behaviourTimer = 0.0f;
//		distanceToTarget = rand() % 250;
//		BehaviourState state = Ongoing;
//		std::cout << "We’re going on an adventure !\n";
//		while (state == Ongoing) {
//			state = rootSequence->Execute(1.0f); //fake dt
//
//		}
//		if (state == Success) {
//			std::cout << "What a successful adventure !\n";
//
//		}
//		else if (state == Failure) {
//			std::cout << "What a waste of time!\n";
//
//		}
//
//	}
//	std::cout << "All done!\n";
//}
//// bring in the behaviour tree


//imported test behaviour tree
//void TestBehaviourTree() {
//	 float behaviourTimer;
//	 float distanceToTarget;
//	 BehaviourAction * findKey = new BehaviourAction("Find Key",[&](float dt, BehaviourState state)->BehaviourState {
//			 if (state == Initialise) {
//				 std::cout << "Looking for a key!\n";
//				 behaviourTimer = rand() % 100;
//				 state = Ongoing;
//				
//			}
//			 else if (state == Ongoing) {
//				 behaviourTimer -= dt;
//				 if (behaviourTimer <= 2.0f) {
//					 std::cout << "Found a key!\n";
//					 return Failure;
//					
//				}
//				
//			}
//			 return state; //will be ’ongoing ’ until success
//			 }
//	);
//
//	 BehaviourAction* goToRoom = new BehaviourAction("Go To Room",
//		  [&](float dt, BehaviourState state)->BehaviourState {
//			  if (state == Initialise) {
//				  std::cout << "Going to the loot room!\n";
//				  state = Ongoing;
//				 
//			 }
//			  else if (state == Ongoing) {
//				  distanceToTarget -= dt;
//				  if (distanceToTarget <= 0.0f) {
//					  std::cout << "Reached room!\n";
//					  return Success;
//					 
//				 }
//				 
//			 }
//			  return state; //will be ’ongoing ’ until success
//			  }
//	 );
//	 BehaviourAction* openDoor = new BehaviourAction("Open Door",
//		  [&](float dt, BehaviourState state)->BehaviourState {
//			  if (state == Initialise) {
//				  std::cout << "Opening Door!\n";
//				  return Success;
//				 
//			 }
//			  return state;
//			  }
//	 );
//	 BehaviourAction* lookForTreasure = new BehaviourAction(
//		  "Look For Treasure",
//		  [&](float dt, BehaviourState state)->BehaviourState {
//			  if (state == Initialise) {
//				  std::cout << "Looking for treasure !\n";
//				  return Ongoing;
//				 
//			 }
//			  else if (state == Ongoing) {
//				  bool found = rand() % 2;
//				  if (found) {
//					  std::cout << "I found some treasure !\n";
//					  return Success;
//					 
//				 }
//				  std::cout << "No treasure in here ...\n";
//				  return Failure;
//				 
//			 }
//			  return state;
//			  }
//	 );
//	 BehaviourAction* lookForItems = new BehaviourAction(
//		  "Look For Items",
//		  [&](float dt, BehaviourState state)->BehaviourState {
//			  if (state == Initialise) {
//				  std::cout << "Looking for items!\n";
//				  return Ongoing;
//				 
//			 }
//			  else if (state == Ongoing) {
//				  bool found = rand() % 2;
//				  if (found) {
//					  std::cout << "I found some items!\n";
//					  return Success;
//					 
//				 }
//				  std::cout << "No items in here ...\n";
//				  return Failure;
//				 
//			 }
//			  return state;
//			  }
//	 );
//	 BehaviourAll* sequence =
//		  new BehaviourAll("Room Sequence");
//	  sequence->AddChild(findKey);
//	  sequence->AddChild(goToRoom);
//	  sequence->AddChild(openDoor);
//	 
//		  BehaviourSelector * selection =
//		  new BehaviourSelector("Loot Selection");
//	  selection->AddChild(lookForTreasure);
//	  selection->AddChild(lookForItems);
//	 
//		  BehaviourSequence * rootSequence =
//		  new BehaviourSequence("Root Sequence");
//	  rootSequence->AddChild(sequence);
//	  rootSequence->AddChild(selection);
//	   //testing 
//	  rootSequence->Reset();
//	  behaviourTimer = 0.0f;
//	  distanceToTarget = rand() % 250;
//	  BehaviourState state = Ongoing;
//
//	  while (state == Ongoing) {
//		  state = rootSequence->Execute(1.0f); //fake dt
//
//	  }
//	  if (state == Success) {
//		  std::cout << "What a successful adventure !\n";
//
//	  }
//	  else if (state == Failure) {
//		  std::cout << "What a waste of time!\n";
//
//	  }
//
//	  }
//imported test behaviour tree


//renderer only update test
void TutorialGame::UpdateGameRenderer(float dt) {
	world->UpdateWorld(dt);
	renderer->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);
}
//renderer only update test

int TutorialGame::UpdateGame(float dt) {//testing returning int

	

	/*if (testStateObject) {
		testStateObject->Update(dt);

	}*/
	//runningtime.timer(dt);
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedObject->GetTransform().GetOrientation() * lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);
	}

	UpdateKeys();

	/*if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}*/

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(1, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));

			Debug::DrawLine(rayPos, Vector3(objClosest->GetRenderObject()->GetTransform()->GetPosition()), Vector4(1, 0, 0.7, 1), 20.0f);

		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	/*Debug::DrawLine(Vector3(world -> GetMainCamera()->GetPosition()) + Vector3(40,0,40), Vector3(0, 50, 100), Vector4(1, 1, 0, 1));*/
	SelectObject();
	MoveSelectedObject();
	movePlayer(goatCharacter);
	//bullet upadet
	
	//End bullet update
	if (Transform::GetCubesCollected() >= 1) {
		for (int i = 0; i < Transform::GetCubesCollected(); ++i) {
			srand(time(0));
			int randomNum = rand() % 100;
			if (randomNum % 2 == 0) {
				randomNum = -randomNum;
			}
			Vector3 pos = Vector3(randomNum, -16, randomNum );
			Debug::DrawLine(Vector3(0, 0, 0), pos, Vector4(1, 0, 1, 1));
			Vector3 sizeCube = Vector3(1.5f, 1.5f, 1.5f);
			AddDestructableCubeToWorld(pos, sizeCube, randomNum, 0.3f);
			Transform::reduceCubesCollected();
		}
	}
	//TestBehaviourTree();
	if (EnemyGoat) {
		//UseBehaviourTree(EnemyGoat,goatCharacter,dt);
		ResetPhantomCube(phantomCubeOutput);
		//amovement->headToPoint(EnemyGoat,Vector3(0,-18,0),10.0f);
		//std::cout << amovement->CanSee(world,EnemyGoat,goatCharacter) << std::endl;
		//amovement->FacePlayer(EnemyGoat, goatCharacter);
		if (goatCharacter->GetTransform().GetPosition().y < -30) {
			if (goatCharacter->GetTransform().getPlayerGoatLives() <= 0) {
				return 1;
			}
			goatCharacter->GetTransform().decreasePlayerGoatLives();
			goatCharacter->GetTransform().SetPosition(Vector3(-10,-15,0));
			//Print::printTopCentre("Only one life left its a shame you arn't a cat", 5.0f);
		}

		if (EnemyGoat->GetTransform().GetPosition().y < -30) {
			if (EnemyGoat->GetTransform().getEnemyGoatLives() <= 0) {
				return 2;
			}
			EnemyGoat->GetTransform().SetPosition(Vector3(10,-15,10));
			EnemyGoat->GetTransform().decreaseEnemyGoatLives();
		}
	}
	//Print::PrintScore(world->GetWorldTransform()->getScore());
	//cout << goatCharacter->getPlayerProjectile()->GetCanFire()<< endl;
	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);

	TutorialGame::addToRunningBulletTime(dt);
	if (TutorialGame::runningBulletTimeLimitTest(goatCharacter->getPlayerProjectile()->GetRateOfFireTransferred())) {
		goatCharacter->getPlayerProjectile()->toggleCanFire();
		//goatCharacter->updateBulletsUsed();
		//unsigned int numObjects = 
		//world->RemoveGameObject();
	}

	/*TutorialGame::addToBulletDeleteTime(dt);
	if (TutorialGame::bulletDeleteTimeTest()) {
		goatCharacter->clearBulletsUsed();
	}*/
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	return 0;
}



void TutorialGame::UpdateKeys() {

	//testing starting gravity
	physics->UseGravity(useGravity);
	physics->SetGravity(Vector3(0, -9.81f, 0));
	//testing starting gravity
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}
	// test variation
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F12)) {
		InitWorldtest();
		selectionObject = nullptr;
	}
	//second test variation 
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		InitWorldtest2();
		selectionObject = nullptr;
	}


	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		TutorialGame::setLockedObjectNull();
	}


	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
		physics->SetGravity(Vector3(0,-9.81f,0));
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}



void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}



void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	/*world->ClearAndErase();
	physics->Clear();
	InitMixedGridWorld(15, 15, 6.0f, 6.0f);
	testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
	InitGameExamples();
	InitDefaultFloor();*/
}


void TutorialGame::InitWorldtest() {
	world->ClearAndErase();
	physics->Clear();

	
	InitMixedGridWorldtest(2, 2, 3.5f, 3.5f);

//	InitGameExamples();
	InitDefaultFloor();
}


void TutorialGame::InitWorldtest2() {
	world->ClearAndErase();
	physics->Clear();
	//Print::printBottomCentre("knock that morally dubious goat off to win ",10);
	//Print::printBottomCentreSecond("And watch out for the wind!!", 10);
	InitMixedGridWorldGoatStrip(2, 2, 3.5f, 3.5f);

	InitDefaultFloorRunway();
}



/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}


GameObject* TutorialGame::AddRunwayToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(30,2,30);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddDestructableCubeToWorld(const Vector3& position, Vector3 dimensions, float dt,float inverseMass) {
	GameObject* cube = new GameObject();
	srand(dt);
	int random = rand()%10;
	int random2 = rand() % 20;
	int random3 = rand() % 13;
	int random4 = rand() % 7;
	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2);
	cube->GetTransform().setDestructable();

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->GetRenderObject()->SetColour(Vector4(random,random2,random3,random4));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetAffectedByGravityFalse();

	world->AddGameObject(cube);

	return cube;
}



GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2);

	

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->GetRenderObject()->SetColour ( { 1, 0.5, 1, 1 });
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}



GameObject* TutorialGame::AddPhantomCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2);


	/*
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->GetRenderObject()->SetColour(Vector4(1,1,0,1));*/

	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->AddForce(Vector3(0,0,420));
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetAffectedByGravityFalse();
	cube->setIsAlpha();
	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::ResetPhantomCube(GameObject* cubeToReset) {
	Vector3 PhantomPosition = cubeToReset->GetTransform().GetPosition();
	if (PhantomPosition.z > 50) {
		srand((unsigned)time(NULL));
		int random = rand()%100;
		int clampedRandom = std::clamp(random,-40,40);
		if (random % 2 == 0)
			clampedRandom = -clampedRandom;
		cubeToReset->GetPhysicsObject()->ClearForces();
		cubeToReset->GetPhysicsObject()->AddForce(Vector3(0, 0, -280));
		cubeToReset->GetTransform().SetPosition(Vector3(clampedRandom, -13, -50));
		cubeToReset->GetPhysicsObject()->AddForce(Vector3(0, 0, 520));
	}
	return;
}


void TutorialGame::setLockedObjectNull() {
	lockedObject = nullptr;
}

void TutorialGame::setLockedObject(GameObject* goatPlayer){
	lockedObject = goatPlayer;
}

GameObject* TutorialGame::AddEnemyGoatToWorld(const Vector3& position) {
	float meshSize = 2.0f;
	float inverseMass = 0.6f;

	GameObject* BadGoat = new GameObject();
	SphereVolume* volume = new SphereVolume(1.7f);
	BadGoat->SetBoundingVolume((CollisionVolume*)volume);
	BadGoat->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	BadGoat->SetRenderObject(new RenderObject(&BadGoat->GetTransform(), charMesh, nullptr, basicShader));
	BadGoat->SetPhysicsObject(new PhysicsObject(&BadGoat->GetTransform(), BadGoat->GetBoundingVolume()));
	BadGoat->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
	BadGoat->GetPhysicsObject()->SetInverseMass(inverseMass);
	BadGoat->GetPhysicsObject()->setCoeficient(0.55f);
	BadGoat->GetPhysicsObject()->InitSphereInertia();
	world->AddGameObject(BadGoat);

	return BadGoat;
}



playerTracking* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 2.0f;
	float inverseMass	= 0.8f;

	playerTracking* character = new playerTracking();
	SphereVolume* volume  = new SphereVolume(1.7f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	character->GetTransform().setGoatID(7);
	character->setImpactAbsorbtionAmount(0.9f);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
	
        character->GetPhysicsObject()->setTorqueFriction(0.005f);
	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->setCoeficient(0.55f);
	character->GetPhysicsObject()->InitSphereInertia();
	setGoatCharacter(character);
	world->AddGameObject(character);

	return character;
}

Projectile* TutorialGame::AddBulletToWorld(playerTracking* playableCharacter) {
	if (playableCharacter->getBulletVectorSize() <= 200) {
		return useNewBullet(playableCharacter);
	}
	
	return useRecycledBulet(playableCharacter);
}


Projectile* TutorialGame::useNewBullet(playerTracking* passedPlayableCharacter) {
	gun wepType = passedPlayableCharacter->getWeponType();

	Projectile* sphere = objectpool->GetObjectW();
	float bulletsInverseMass = sphere->getWeight();
	float radius = sphere->getProjectileRadius();
	Vector3 playerDirectionVector = (passedPlayableCharacter->GetTransform().GetOrientation() * Vector3 { 0, 0, -1 });
	Vector3 sphereSize = { radius,radius,radius };
	Vector3 position = passedPlayableCharacter->GetTransform().GetPosition();
	SphereVolume* volume = new SphereVolume(radius);
	sphere->setBulletDirectionVector(playerDirectionVector);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize);
	sphere->GetTransform().SetPosition(position - Vector3{ 0,0,3 });

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));
	sphere->GetRenderObject()->SetColour(passedPlayableCharacter->getPaintColor());
	PhysicsObject* physicsBullet = sphere->GetPhysicsObject();
	if (!sphere->ProjectileAffectedByGravity() || true) {
		physicsBullet->SetAffectedByGravityFalse();
	}
	sphere->GetPhysicsObject()->SetInverseMass(bulletsInverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);
	return sphere;

}


Projectile* TutorialGame::useRecycledBulet(playerTracking* passedPlayableCharacter) {
	gun wepType = passedPlayableCharacter->getWeponType();

	Projectile* sphere = passedPlayableCharacter->reuseBullet();
	float bulletsInverseMass = sphere->getWeight();
	float radius = sphere->getProjectileRadius();
	Vector3 playerDirectionVector = (passedPlayableCharacter->GetTransform().GetOrientation() * Vector3 { 0, 0, -1 });
	Vector3 sphereSize = { radius,radius,radius };
	Vector3 position = passedPlayableCharacter->GetTransform().GetPosition();
	SphereVolume* volume = new SphereVolume(radius);
	sphere->setBulletDirectionVector(playerDirectionVector);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize);
	sphere->GetTransform().SetPosition(position - Vector3{ 0,0,3 });

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));
	sphere->GetPhysicsObject()->SetLinearVelocity(Vector3{ 0,0,0 });
	sphere->GetRenderObject()->SetColour(Vector4(0,1,1,1));
	PhysicsObject* physicsBullet = sphere->GetPhysicsObject();
	if (!sphere->ProjectileAffectedByGravity() || true) {
		physicsBullet->SetAffectedByGravityFalse();
	}
	sphere->GetPhysicsObject()->SetInverseMass(bulletsInverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);
	return sphere;
}




Projectile* TutorialGame::FireBullet(playerTracking* selectedPlayerCharacter) {
	Projectile* loadedBullet = useNewBullet(selectedPlayerCharacter);
	//selectedPlayerCharacter->addToBulletsUsed(loadedBullet);
	PhysicsObject* physicsShot = loadedBullet->GetPhysicsObject();
	physicsShot->SetLayerID(); // set Id so bullets cannot collied with each other and players.
	//loadedBullet->GetTransform().setDestructable();
	physicsShot->SetLinearVelocity({0,0,0});
	physicsShot->ClearForces();
	float const startingForce = loadedBullet->getPojectilePropultionForce();
	Vector3 firingDirectionVector = loadedBullet->getBulletDirectionVector();
	Vector3 firingDirectionVectorWithForce = firingDirectionVector * startingForce;
	physicsShot->AddForce(firingDirectionVectorWithForce);
	

	//testing bullet vector removal
	/*if (selectedPlayerCharacter->getBulletVectorSize() > 10) {
		selectedPlayerCharacter->clearBulletsUsed();
	}*/
	//testing bullet vector removal
	return loadedBullet;
}


void TutorialGame::setGoatCharacter(playerTracking* assignCharcter) {
	goatCharacter = assignCharcter;
}

void TutorialGame::setEnemyGoat(GameObject* assignCharcter) {
	EnemyGoat = assignCharcter;
}


void TutorialGame::movePlayer(playerTracking* unitGoat) {
	unitGoat->GetRenderObject()->SetColour(Vector4(0.1f, 0.2f, 0.4f, 1));
	PhysicsObject* goatPhysicsObject = unitGoat->GetPhysicsObject();
	Transform* goatTransform = &(unitGoat->GetTransform());
	//jumping raycast test
	Vector3 rayPos;
	Vector3 rayDir;
	rayDir = unitGoat->GetTransform().GetOrientation() * Vector3(0, -1, 0);
	rayPos = unitGoat->GetTransform().GetPosition();

	Ray r = Ray(rayPos, rayDir);

	RayCollision grounded;
	if (world->Raycast(r, grounded, true)) {
		/*if (objClosest) {
			objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
		}*/
		objClosest = (playerTracking*)grounded.node;
		//objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		float distanceFromPlatform = abs((unitGoat->GetTransform().GetPosition().y) - (objClosest->GetTransform().GetPosition().y));
		if (distanceFromPlatform > 5) {
			goatPhysicsObject->setCanJumpFalse();
		}
		//Debug::DrawLine(rayPos, Vector3(objClosest->GetRenderObject()->GetTransform()->GetPosition()), Vector4(1, 0, 0.7, 1), 20.0f);
		//std::cout << distanceFromPlatform << std::endl;
		if (distanceFromPlatform < 4.0f) {
			goatPhysicsObject->setCanJumpTrue();


		}
	}
	//jumping raycast test
	if ((Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::H)) && (goatPhysicsObject->GetCanJump())) {
		//std::cout << "hi we reached this point " << std::endl;
		goatPhysicsObject->AddForce(Vector3(0.0f, 1100.0f, 0.0f));
	};

	if ((Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::F))) {
		if (unitGoat->getPlayerProjectile()->GetCanFire()) {
			FireBullet(unitGoat);
			unitGoat->getPlayerProjectile()->toggleCanFire();
		};
	};

	if ((Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::W)) || (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::W))) {
		goatPhysicsObject->AddForce(((unitGoat->GetTransform().GetOrientation()) * (Vector3(0, 0, 1)).Normalised()) * -20.0f);
	}

	if ((Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::S)) || (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::S))) {
		goatPhysicsObject->AddForce(((unitGoat->GetTransform().GetOrientation()) * (Vector3(0, 0, 1)).Normalised()) * 10.0f);
	}

	if ((Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::D)) || (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::D))) {
		goatPhysicsObject->AddTorque(Vector3(0, -2.0, 0));
	}
	if ((Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::A)) || (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::A))) {
		goatPhysicsObject->AddTorque(Vector3(0, 2.0, 0));
	}

}




GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 10.0f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

//trying phantom stuff
GameObject* TutorialGame:: AddPhantomBoxToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* GostBox = new GameObject();

	AABBVolume* boxVolume = new AABBVolume(Vector3(0.1f, 0.1f, 0.1f));
	GostBox->SetBoundingVolume((CollisionVolume*)boxVolume);

	GostBox->GetTransform().SetScale(Vector3(0.1f, 0.1f, 0.1f)).SetPosition(position);

	GostBox->SetPhysicsObject(new PhysicsObject(&GostBox->GetTransform(), GostBox->GetBoundingVolume()));

	GostBox->GetPhysicsObject()->SetInverseMass(inverseMass);
	GostBox->GetPhysicsObject()->InitCubeInertia();
	world->AddGameObject(GostBox);

	return GostBox;
}
//trying phantom stuff



GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}

void TutorialGame::InitDefaultFloorRunway() {
	AddRunwayToWorld(Vector3(0, -20, 0));
}

void TutorialGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}



//origonal
void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 2.0f;
	Vector3 cubeDims = Vector3(1.5f, 1.5f, 1.5f);
	playerTracking* player1 = AddPlayerToWorld(Vector3(-10, -17, 0));
	movePlayer(player1);
	setLockedObject(player1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			/*Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);*/
			Vector3 position = Vector3(x * colSpacing, -16.0f, z * rowSpacing);

			if (rand() % 2) {
				AddDestructableCubeToWorld(position, cubeDims,float(z+0.5f),0.3f);
			}
			else {
				//AddSphereToWorld(position, sphereRadius,0.5f);
			}
		}
	}
}

// modified 
void TutorialGame::InitMixedGridWorldtest(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);
//	chainBallTest();
	StateGameObject* AddStateObjectToWorld(const Vector3& position );
	playerTracking* player1 = AddPlayerToWorld(Vector3(-10, -10, 0));
	movePlayer(player1);
	setLockedObject(player1);
			Vector3 position1 = Vector3(0 * colSpacing, 10.0f, 0 * rowSpacing);
			Vector3 position2 = Vector3(1 * colSpacing, 10.0f, 1 * -rowSpacing);

				AddCubeToWorld(position1, cubeDims);
		
				//AddSphereToWorld(position2, sphereRadius);
			
}


void TutorialGame::InitMixedGridWorldGoatStrip(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);
	Vector3 PhantomCubeDims = Vector3(10, 4, 4);
	chainBallTest();
	EnemyGoat = AddEnemyGoatToWorld(Vector3(10,-15,10));
	playerTracking* player1 = AddPlayerToWorld(Vector3(-10, -10, 0));
	movePlayer(player1);
	setLockedObject(player1);
	Vector3 position1 = Vector3(0 * colSpacing, 10.0f, 0 * rowSpacing);
	Vector3 position2 = Vector3(1 * colSpacing, 10.0f, 1 * -rowSpacing);
	Vector3 poistion3 = Vector3(0, -13, -50);

	//AddCubeToWorld(position1, cubeDims);

	//AddSphereToWorld(position2, sphereRadius);
	 phantomCubeOutput = AddPhantomCubeToWorld(poistion3,PhantomCubeDims,10.0f);
	std::cout << phantomCubeOutput->GetTransform().GetPosition() << std::endl;
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 0.0f, z * rowSpacing);
			//AddDestructableCubeToWorld(position, cubeDims, 0.3f);


		}
	}
	

}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		//Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 0.6, 0.8, 1));
				//test force application 
				//selectionObject->GetPhysicsObject()->AddForce(Vector3(100, 100, 100));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		//Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	//Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}



//void TutorialGame::MoveSelectedObject() {
//	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
//	//Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
//	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
//
//	if (!selectionObject) {
//		return;//we haven't selected anything!
//	}
//	//Push the selected object!
//	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
//		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
//
//		RayCollision closestCollision;
//		if (world->Raycast(ray, closestCollision, true)) {
//			if (closestCollision.node == selectionObject) {
//				selectionObject->GetPhysicsObject()->AddForce(ray.GetDirection() * forceMagnitude);
//			}
//		}
//	}
//}
//
//






void TutorialGame::BridgeConstraintTest() {
	Vector3 CubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5; // mass of the middle pieces
	int numLinks = 10;
	float maxDistance = 30; //constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(50, 100, 50);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), CubeSize, 0);

	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), CubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), CubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);

		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}


void TutorialGame::chainBallTest() {
	//cube pole
	Vector3 cubeSize = Vector3(0.1f, 1.0f, 0.1f);
	float inverseCubeMass = 0.0f;
	float cubeDistance = 0.0f;
	Vector3 cubeStart = Vector3(0, -20, 0);
	for (int i = 0; i < 7; ++i) {
		AddCubeToWorld(cubeStart + Vector3(0, i, 0), cubeSize, inverseCubeMass);
	}
	//sphere properties
	float smallSphereSize = 0.1f;
	float LargeSphereSize = 0.5f;
	float boltSphereInversMass = 0.0f;
	float smallSphereInverseMass = 100.0f;
	float largeSphereInverseMass = 50.0f;
	float maxDistance = 0.21f;
	int sphereLinks = 10;
	Vector3 topOfPolePosition = Vector3(0, -12.5, 0);
	//add the bolt sphere at the top of the cubes
	AddSphereToWorld(Vector3(0,-12.5,0),LargeSphereSize,boltSphereInversMass);
    //first chain link
	GameObject* start = AddSphereToWorld(Vector3(0, -12.5, 0), LargeSphereSize, boltSphereInversMass);
		//AddSphereToWorld(topOfPolePosition + Vector3(0.4f,-0.4, 0), smallSphereSize, smallSphereInverseMass);
	//GameObject* end = AddSphereToWorld(topOfPolePosition + Vector3(0.4f, -0.4, 0), smallSphereSize, 0);

	GameObject* previous = start;
	for (int i = 0; i < sphereLinks; ++i) {
		if (i == 0) {
			GameObject* block = AddSphereToWorld((previous->GetTransform().GetPosition()) + Vector3(0.4f, -((smallSphereSize * 1.0)-LargeSphereSize), 0), smallSphereSize, smallSphereInverseMass);
			PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
			((SphereVolume*)(block->GetBoundingVolume()))->toggleLayerActive();
			world->AddConstraint(constraint);
			previous = block;
		}
		else if (i == sphereLinks - 1)
		{
			GameObject* block = AddSphereToWorld((previous->GetTransform().GetPosition()) + Vector3(0.4f, -((smallSphereSize * 1.0) - LargeSphereSize), 0), LargeSphereSize, largeSphereInverseMass);
			PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
			((SphereVolume*)(block->GetBoundingVolume()))->toggleLayerActive();
			world->AddConstraint(constraint);
			previous = block;
		}
		
		else{
			GameObject* block = AddSphereToWorld((previous->GetTransform().GetPosition()) + Vector3(0.0f, -((smallSphereSize * 2.0)), 0), smallSphereSize, smallSphereInverseMass);
			PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
			((SphereVolume*)(block->GetBoundingVolume()))->toggleLayerActive();
			world->AddConstraint(constraint);
			previous = block;
		}
	}
	/*PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);*/
}



StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* object = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	object->SetBoundingVolume((CollisionVolume*)volume);
	object->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	object->SetRenderObject(new RenderObject(&object->GetTransform(), sphereMesh, nullptr, basicShader));
	object->SetPhysicsObject(new PhysicsObject(&object->GetTransform(), object->GetBoundingVolume()));

	object->GetPhysicsObject()->SetInverseMass(1.0f);
	object->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(object);

	return object;

}



