#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include <minmax.h>
#include "Player.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	InitialiseAssets();
	
}

void TutorialGame::InitQuadTexture() {
	std::array<float, 1280 * 720 * 3>* data = new std::array<float, 1280 * 720 * 3>();//todo dont hardcode
	quadTex = new OGLTexture();
	renderer->quad = new RenderObject(nullptr, OGLMesh::GenerateQuadWithIndices(), quadTex, quadShader);
	for (int i = 0; i < 1280*720*3; i++)
	{ 
		data->at(i) = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}
	
	glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)quadTex)->GetObjectID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1280, 720, 0, GL_RGB, GL_FLOAT, data->data());
	return;

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

	//this was me
	computeShader = new OGLComputeShader("compute.glsl");
	quadShader = new OGLShader("quad.vert", "quad.frag");
	
	InitQuadTexture();
	


	InitCamera();
	InitWorld();
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

	//todo delete texture array
	//todo delete compute shader
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (player != nullptr) {
		Vector3 objPos = player->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!
		world->GetMainCamera()->SetTargetPosition(objPos);
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
	float testFloat = float(1000) / float(55);
	if (1000 * dt > testFloat)std::cout << "fps drop\n";
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
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
	if (Window::GetMouse()->ButtonPressed(MouseButtons::LEFT))
	{
		AddPaintBall(world->GetMainCamera()->GetPosition(), Vector3(10000, 0, 0));
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		for (int i = 0; i < 1; i++)
		{
			Vector3 halfDims = worldFloor->GetTransform().GetScale() / 2;
			float randX = (rand() % 200) - 100;
			float randY = (rand() % 200) - 100;
			Vector3 randVec(randX, 2, randY);
			Vector2 center;
			int radius = 10;
			int startIndex, numInts, leftS, rightS, topT, bottomT;
			worldFloor->ApplyPaintAtPosition(randVec, halfDims, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
			RunComputeShader(worldFloor,200,200, leftS, rightS, topT, bottomT, radius, center);
		}
		
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F)) {
		renderer->renderFullScreenQuad = !renderer->renderFullScreenQuad;

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

	world->ClearAndErase();
	physics->Clear();


	AddPlayerToWorld(Vector3(0, 1, 0)); //Init Player
	InitDefaultFloor();
}

void TutorialGame::RunComputeShader(GameObject* floor,int width, int height, int leftS, int rightS, int topT, int bottomT, int radius, Vector2 center) {
	computeShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, ((OGLTexture*)floor->GetRenderObject()->GetDefaultTexture())->GetObjectID(), 0, GL_FALSE, NULL, GL_WRITE_ONLY, GL_R8);



	int widthLocation = glGetUniformLocation(computeShader->GetProgramID(), "width");
	glUniform1i(widthLocation, width * TEXTURE_DENSITY);

	int heightLocation = glGetUniformLocation(computeShader->GetProgramID(), "height");
	glUniform1i(heightLocation, height * TEXTURE_DENSITY);

	int leftSLocation = glGetUniformLocation(computeShader->GetProgramID(), "leftS");
	glUniform1i(leftSLocation, leftS);

	int rightSLocation = glGetUniformLocation(computeShader->GetProgramID(), "rightS");
	glUniform1i(rightSLocation, rightS);

	int topTLocation = glGetUniformLocation(computeShader->GetProgramID(), "topT");
	glUniform1i(topTLocation, topT);

	int bottomTLocation = glGetUniformLocation(computeShader->GetProgramID(), "bottomT");
	glUniform1i(bottomTLocation, bottomT);

	int radiusLocation = glGetUniformLocation(computeShader->GetProgramID(), "radius");
	glUniform1i(radiusLocation, radius * TEXTURE_DENSITY);

	int centerLocation = glGetUniformLocation(computeShader->GetProgramID(), "center");
	glUniform2i(centerLocation, center.x,center.y);

	computeShader->Execute(rightS-leftS, bottomT-topT, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void TutorialGame::InitPaintableTextureOnObject(GameObject* object) {
	OGLTexture* tex = new OGLTexture();
	glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)tex)->GetObjectID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, object->GetTransform().GetScale().x * TEXTURE_DENSITY, object->GetTransform().GetScale().z * TEXTURE_DENSITY, 0, GL_RED, GL_BYTE, nullptr);
	object->SetRenderObject(new RenderObject(&object->GetTransform(), cubeMesh, tex, basicShader));
}
/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(100, 2, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));
	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	
	floor->isPaintable = true;
	
	srand(time(0));
	int test;

	InitPaintableTextureOnObject(floor);

	int radius = 10;
	int startIndex, numInts, leftS,rightS,topT,bottomT;
	Vector2 center;
	floor->ApplyPaintAtPosition(Vector3(-50, 4, 0), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center);
	floor->ApplyPaintAtPosition(Vector3(50, 4, 0), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center);
	floor->ApplyPaintAtPosition(Vector3(0, 4, 50), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center);
	floor->ApplyPaintAtPosition(Vector3(0, 4, -50), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center);

	
	/*for (int x = 0; x < 10000; x++)
	{
		(*paintDataPtr)->at(x) = 1;
	}*/
	//(*paintDataPtr)->fill(1);

	
	
	

	

	//floor->GetRenderObject()->texID = floor->texture;

	/*glGenBuffers(1, &(floor->ssbo));
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, floor->ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_WORLD_UNITS_SQUARED * TEXTURE_DENSITY * TEXTURE_DENSITY * sizeof(int), (*paintDataPtr)->data(), GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, floor->ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);
	worldFloor = floor;
	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/

#pragma region AddObject Template
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

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	this->player = new Player();
	SphereVolume* volume = new SphereVolume(1.0f);

	this->player->SetBoundingVolume((CollisionVolume*)volume);

	this->player->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	this->player->SetRenderObject(new RenderObject(&this->player->GetTransform(), charMesh, nullptr, basicShader));
	this->player->SetPhysicsObject(new PhysicsObject(&this->player->GetTransform(), this->player->GetBoundingVolume()));

	this->player->GetPhysicsObject()->SetInverseMass(inverseMass);
	this->player->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(this->player);


}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

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
#pragma endregion



GameObject* NCL::CSC8503::TutorialGame::AddPaintBall(const Vector3& position, Vector3 direction)
{
	GameObject* paintball = new GameObject();
	SphereVolume* volume = new SphereVolume(1.0f);
	paintball->SetBoundingVolume((CollisionVolume*)volume);
	paintball->GetTransform().SetScale(Vector3(2, 2, 2)).SetPosition(position);

	paintball->SetRenderObject(new RenderObject(&paintball->GetTransform(), sphereMesh, nullptr, basicShader));
	paintball->SetPhysicsObject(new PhysicsObject(&paintball->GetTransform(), paintball->GetBoundingVolume()));
	paintball->GetPhysicsObject()->AddForce(direction);

	world->AddGameObject(paintball);

	return paintball;
	
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
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

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
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
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
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
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
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
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
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




