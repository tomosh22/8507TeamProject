#include<iostream>
#include<cmath>
#include <chrono>
#include <corecrt_math_defines.h>

#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"

#include "Projectile.h"

#include<iostream>
#include <chrono>
#include "RespawnPoint.h"

using namespace NCL;
using namespace CSC8503;

//#define TRI_DEBUG
//#define OLD_PAINT
//#define DEBUG_SHADOW


TutorialGame::TutorialGame(GameWorld* gameWorld) :
	world(gameWorld)
{
	
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(Window::GetWindow());
#endif

	physics	= new PhysicsSystem(*world);
	propSystem = new PropSystem(world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	testStateObject = nullptr;

	objectpool = new ObjectPool<Projectile>();

	InitialiseAssets();

	SetUpTriangleSSBOAndDataTexture();

	Vector3 triA(2, 2, 1);
	Vector3 triB(7, 4, 1);
	Vector3 triC(5, 9, 1);
	Vector3 point(3, 3, 1);
	Vector3 uvw = PhysicsObject::WorldSpaceToBarycentricCoords(point,triA,triB,triC);
	std::cout << uvw;


	
	
	//this was me
	glGenTextures(1, &depthBufferTex);
	glBindTexture(GL_TEXTURE_2D, depthBufferTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_DEPTH_COMPONENT,
		renderer->GetWindowWidth(),
		renderer->GetWindowHeight(),
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		0);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	testSphereCenter = Vector3(320,190,230);
	testSphereRadius = 10;
	/*
	renderer->imguiptrs.testSphereCenter = &testSphereCenter;
	renderer->imguiptrs.testSphereRadius = &testSphereRadius;

	
	renderer->imguiptrs.newMethod = &renderer->newMethod;

	renderer->imguiptrs.rayMarchBool = &rayMarch;*/

	glGenBuffers(1, &triangleBoolSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleBoolSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, highestTriCount * sizeof(int), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &triangleRasteriseSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleRasteriseSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, highestTriCount * sizeof(int) * 4, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &triangleRasteriseSSBOSecondShader);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleRasteriseSSBOSecondShader);
	glBufferData(GL_SHADER_STORAGE_BUFFER, highestTriCount * sizeof(int) * 3, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	for (int i = 0; i < 1000 * 1000; i++)
	{
		zeros[i] = 0;
	}

	glGenBuffers(1, &tempSSBO);


	renderer->crosshair = new RenderObject(nullptr,  OGLMesh::GenerateCrossHair(), nullptr, renderer->debugShader);

	playerMaterial = new MeshMaterial("Character/Character.mat");



	currentFrame = 0;
	frameTime = 0.0f;
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete propSystem;


	delete objectpool;

	//todo delete texture array
	//todo delete compute shader
}

void TutorialGame::InitQuadTexture() {
	int width = (renderer->GetWindowWidth());
	int height = (renderer->GetWindowHeight());
	//std::array<float, 1280 * 720 * 4>* data = new std::array<float, 1280 * 720 * 4>();//todo dont hardcode
	
	//for (int i = 0; i < 1280*720*4; i++)
	//{ 
	//	data->at(i) = (float)rand() / (float)RAND_MAX;
	//}

	//todo maybe move this somewhere else? still somewhat related

	//may need to adjust if fps tanks
	/*
	renderer->imguiptrs.rayMarchMaxSteps = &maxSteps;
	renderer->imguiptrs.rayMarchHitDistance = &hitDistance;
	renderer->imguiptrs.rayMarchNoHitDistance = &noHitDistance;
	renderer->imguiptrs.debugValue = &debugValue;
	renderer->imguiptrs.depthTest = &rayMarchDepthTest;*/
	//renderer->imguiptrs.currentTeamInt = &currentTeamInt;
}

//raymarching


void TutorialGame::InitialiseAssets() {
	std::vector<MeshGeometry*> meshes;
	cubeMesh = renderer->LoadMesh("cube.msh", &meshes);
	sphereMesh = renderer->LoadMesh("sphere.msh", &meshes);
	charMesh = renderer->LoadMesh("goat.msh", &meshes);
	enemyMesh = renderer->LoadMesh("Keeper.msh", &meshes);
	bonusMesh = renderer->LoadMesh("apple.msh", &meshes);
	capsuleMesh = renderer->LoadMesh("capsule.msh", &meshes);

	//this was me
	triangleMesh = OGLMesh::GenerateTriangleWithIndices();
	monkeyMesh = renderer->LoadMesh("newMonkey.msh", &meshes);
	floorMesh = renderer->LoadMesh("Corridor_Floor_Basic.msh", &meshes);
	maxMesh = renderer->LoadMesh("Rig_Maximilian.msh", &meshes);
	basicWallMesh = renderer->LoadMesh("corridor_Wall_Straight_Mid_end_L.msh", &meshes);
	bunnyMesh = renderer->LoadMesh("bunny.msh", &meshes);

	playerMesh = renderer->LoadMesh("Character/Character.msh", &meshes);

	playerMesh->SetPrimitiveType(GeometryPrimitive::Triangles);

	for (MeshGeometry*& mesh : meshes) {
		if (mesh->GetIndexData().size() == 0) std::cout << "mesh doesn't use indices, could be a problem\n";
		if (mesh->GetIndexCount() / 3 > (unsigned int)highestTriCount) {
			highestTriCount = mesh->GetIndexCount() / 3;
		}
	}
#pragma region debuggingSphereTriangleCollisions
	/*MESH_TRIANGLES_AND_UVS tris = capsuleMesh->GetAllTrianglesAndUVs();
	std::vector<std::array<float, 4>> results{};
	for (int i = 0; i < tris.size(); i++)
	{
		std::array<Vector3, 3> tri = tris[i];
		std::array<float, 4> result;
		Vector3 intersectionPoint;
		if (SphereTriangleIntersection(Vector3(), 1, tri[0], tri[1], tri[2], intersectionPoint)) {
			Vector3 barycentric = PhysicsObject::WorldSpaceToBarycentricCoords(intersectionPoint, tri[0], tri[1], tri[2]);
			result[0] = barycentric.x;
			result[1] = barycentric.y;
			result[2] = barycentric.z;
			result[3] = 1;
		}
		else {
			result[0] = 0;
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
		}
		results.push_back(result);
	}*/
#pragma endregion

	basicTex	= renderer->LoadTexture("checkerboard.png");
	wallTex		= renderer->LoadTexture("corridor_wall_c.tga");
	
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	basicShader = renderer->LoadShader("scene.vert", "scene.frag", "scene.tesc", "scene.tese");

	
	metalTex = renderer->LoadTexture("metal.png");

	testBumpTex = renderer->LoadTexture("testBump.jpg");


	

	


	//this was me
	computeShader = new OGLComputeShader("compute.glsl");

	triComputeShader = new OGLComputeShader("tris.comp");
	triRasteriseShader = new OGLComputeShader("rasteriseTriangle.comp");

	characterShader = new OGLShader("SkinningVertex.vert", "SkinningFrag.frag");

	InitQuadTexture();

	InitCamera();
	InitWorld();
}

void TutorialGame::UpdateWorldCamera(float dt) {
	if (!playerObject) {
		world->GetMainCamera()->UpdateCamera(dt);
		return;
	}
	CameraLockOnPlayer();
}

void TutorialGame::CameraLockOnPlayer() {
	if (!playerObject) { return; }
	Vector3 objPos = playerObject->GetTransform().GetPosition();
	//find object orientation
	float yrot = playerObject->GetTransform().GetOrientation().ToEuler().y;
	//set camera position
	Vector3 offSet = Vector3(viewOffset.x * cosf((yrot + 270.0f) * M_PI / 180), viewOffset.y, viewOffset.z * sinf((yrot - 270.0f) * M_PI / 180));

	Vector3 camPos = objPos + offSet + Vector3::Cross(Vector3(0.0f, 1.0f, 0.0f), offSet).Normalised() * 2.0f;
	//targeting in front of the object
	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos - offSet * Vector3(10.0f, 0.0f, 10.0f), Vector3(0, 1, 0));
	Matrix4 modelMat = temp.Inverse();
	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	world->GetMainCamera()->SetPosition(camPos);
	world->GetMainCamera()->SetYaw(angles.y);
	world->GetMainCamera()->UpdateObjectViewPitch();
}

void TutorialGame::RayCast() {
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
}

void TutorialGame::UpdateGame(float dt) {
#ifdef DEBUG_SHADOW
	renderer->lightPosition = world->GetMainCamera()->GetPosition();
#endif

	switch (gameMode) {
	case GAME_MODE_DEFAULT:
	{
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
		return;
	}
	case GAME_MODE_GRAPHIC_TEST:
	{
		//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 4, "cube");
		//DispatchComputeShaderForEachTriangle(testCube);
		//glPopDebugGroup();
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 6, "monkey");
		DispatchComputeShaderForEachTriangle(monkey, testSphereCenter, testSphereRadius, TEAM_RED);
		glPopDebugGroup();
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "floor");
		DispatchComputeShaderForEachTriangle(floor, testSphereCenter, testSphereRadius, TEAM_RED);
		glPopDebugGroup();
		//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "walls");
		//for (GameObject*& wall : walls) {
			//DispatchComputeShaderForEachTriangle(wall);
		//}
		//glPopDebugGroup();
		RayCast();
		break;
	}
	case GAME_MODE_SELECT_TEAM:
	{
		int teamID = SelectTeam();
		if (teamID != 0) {
			gameMode = GAME_MODE_ONLINE_GAME;
			InitOnlineGame(teamID);
			return;
		}
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
		return;
	}
	case GAME_MODE_ONLINE_GAME:
	{
		//Host player selects the number of players
		if (0 == playerNum && 0 == playerObject->GetNetworkId() && playerObject->IsOnline()) {
			Debug::Print("Please choose your team.", Vector2(30, 30));
			Debug::Print("1. 2 Players.", Vector2(30, 35) );
			Debug::Print("2. 4 Players.", Vector2(30, 40));
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
				playerNum = TWO_PLAYERS;
			}
			else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
				playerNum = FOUR_PLAYERS;
			}
			if (playerNum != 0) {
				playerObject->WriteActionMessage(Select_Player_Mode, playerNum);
			}

			renderer->Update(dt);
			renderer->Render();
			Debug::UpdateRenderables(dt);
			return;
		}
	}
	case GAME_MODE_SINGLE_GAME:
	{
		frameTime -= dt;
		UpdateAnimations(dt);
		break;
	}
	default:
		std::cout << "Game mode error" << std::endl;
		return;
	}

	timePassed += dt;

//	if (rayMarch)DispatchComputeShaderForEachPixel();

	UpdateWorldCamera(dt);

	ControlPlayer(dt);

	UpdateKeys();

	world->UpdateWorld(dt);

	renderer->Update(dt);
	physics->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);


	
	
	//if (GAME_MODE_GRAPHIC_TEST == gameMode) {
	//	UpdateRayMarchSpheres();
	//	SendRayMarchData();
	//}
}

int TutorialGame::SelectTeam() {
	Debug::Print("Please choose your team.", Vector2(30, 30));
	Debug::Print("1. RED.", Vector2(30, 35), Debug::RED);
	Debug::Print("2. BLUE.", Vector2(30, 40), Debug::BLUE);
	Debug::Print("3. GREEN.", Vector2(30, 45), Debug::GREEN);
	Debug::Print("4. YELLOW.", Vector2(30, 50), Debug::YELLOW);
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
		return TEAM_RED;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
		return TEAM_BLUE;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM3)) {
		return TEAM_GREEN;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM4)) {
		return TEAM_YELLOW;
	}
	return 0;
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
		physics->SetGravity(Vector3(0, -9.81f, 0));
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->SetShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->SetShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->SetShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->SetShuffleObjects(false);
	}

  if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		float randX = (float)((rand() % 200) - 100);
		float randZ = (float)((rand() % 200) - 100);
		Vector3 randVec(randX, 2, randZ);
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "floor");
		DispatchComputeShaderForEachTriangle(floor, {randX,5,randZ},10, TEAM_DEFAULT);
		glPopDebugGroup();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F)) {
		renderer->renderFullScreenQuad = !renderer->renderFullScreenQuad;

	}
}

void TutorialGame::ControlPlayer(float dt) {
	if (pause) { return; }
	if (!playerObject) { return; }

	Transform& transform = playerObject->GetTransform();

	Vector3 fwdAxis = transform.GetDirVector().Normalised();
	Vector3 rightAxis = Vector3::Cross(Vector3(0.0f, -1.0f, 0.0f), fwdAxis);

	//orientation
	float dirVal = Window::GetMouse()->GetRelativePosition().x;
	while (180.0f <= dirVal) { dirVal -= 180.0f; }
	Quaternion orientation = transform.GetOrientation();
	orientation = orientation + (Quaternion(Vector3(0, -dirVal *0.002f, 0), 0.0f) * orientation);
	transform.SetOrientation(orientation.Normalised());
	//speed
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT) && playerObject->CanJump(floor)) {
		playerObject->SpeedUp();
	}
	else 
	{
		playerObject->SpeedDown();
	}
	float speed = playerObject->GetSpeed();
	Vector3 position = transform.GetPosition();
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		if (playerObject->GetOnLadder() && Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
			playerObject->GetTransform().SetPosition(playerObject->GetTransform().GetPosition() + Vector3{0,1,0} *dt * speed);
			std::cout << "going up " << std::endl;
		}
		else
		{
			playerObject->GetTransform().SetPosition(playerObject->GetTransform().GetPosition() + fwdAxis * dt * speed);
			playerObject->TransferAnimation("MoveF");
			std::cout << "going forward " << std::endl;
		}
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		playerObject->GetTransform().SetPosition(playerObject->GetTransform().GetPosition() - fwdAxis * dt * speed);
		playerObject->TransferAnimation("MoveB");
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		playerObject->GetTransform().SetPosition(playerObject->GetTransform().GetPosition() - rightAxis * dt * speed);
		playerObject->TransferAnimation("MoveL");
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		playerObject->GetTransform().SetPosition(playerObject->GetTransform().GetPosition() + rightAxis * dt * speed);
		playerObject->TransferAnimation("MoveR");
	}
	if(!Window::GetKeyboard()->KeyDown(KeyboardKeys::W)&& !Window::GetKeyboard()->KeyDown(KeyboardKeys::S)
		&&!Window::GetKeyboard()->KeyDown(KeyboardKeys::A)&& !Window::GetKeyboard()->KeyDown(KeyboardKeys::D))
	{
		playerObject->TransferAnimation("Idle");
	}
	//jump
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE) && playerObject->CanJump(floor)) {
		playerObject->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, PLAYER_JUMP_FORCE, 0));
	}
	//switch weapon
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::TAB)) {
		playerObject->SwitchWeapon();
		playerObject->WriteActionMessage(PLAYER_ACTION_SWITCH_WEAPON);
	}
	//Aiming
	if (Window::GetMouse()->ButtonHeld(MouseButtons::RIGHT)){
		renderer->drawCrosshair = true;
		playerObject->UpdateAimPosition(world->GetMainCamera());
		//shoot
		if (Window::GetMouse()->ButtonHeld(MouseButtons::LEFT) && playerObject->CanShoot()) {
			playerObject->StartShooting(playerObject->GetAimedTarget());
			playerObject->WriteActionMessage(PLAYER_ACTION_SHOOT);
		}
	}
	else 
	{
		renderer->drawCrosshair = false;
	}
	playerObject->PrintPlayerInfo();
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

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
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
	gameMode = GAME_MODE_DEFAULT;
}

void TutorialGame::InitGraphicTest() {
	renderer->drawCrosshair = true;
	world->ClearAndErase();
	physics->Clear();

	//InitDefaultFloor();

	//position is irrelevant at this point in testing as im overriding position later
	//for raymarching

	
	

	//testCube = AddCubeToWorld(Vector3(), Vector3(100, 100, 100));
	floor = AddFloorToWorld({ 0,0,0 }, { 100,1,100 });
	floor->SetName(std::string("floor"));

	//testTriangle = AddDebugTriangleToWorld({ 0,200,0 });

	monkey = AddMonkeyToWorld({ 0,20,100 }, { 5,5,5 },false);
	monkey->SetName(std::string("monkey"));
	monkey->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, 180, 0));
	monkey->GetRenderObject()->useHeightMap = true;
	walls.push_back(AddFloorToWorld({ 0,25,-50 }, {100,1,25},true));
	walls.back()->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(90, 0, 0));
	walls.back()->SetName(std::string("back"));

	bunny = AddBunnyToWorld({ 0,20,50 }, { 5,5,5 },false);
//	bunny->GetRenderObject()->pbrTextures = rockPBR;
	bunny->GetRenderObject()->useHeightMap = true;

	//max = AddMaxToWorld({ 10,10,-10 }, { 10,10,10 });

#ifdef TRI_DEBUG
	//AddDebugTriangleInfoToObject(testCube);
	AddDebugTriangleInfoToObject(monkey);
	AddDebugTriangleInfoToObject(floor);
	//AddDebugTriangleInfoToObject(max);
	for (GameObject*& wall : walls) {
		AddDebugTriangleInfoToObject(wall);
	}
#endif
	floor->GetRenderObject()->useHeightMap = true;
	return;
}

void TutorialGame::InitSingleGameMode() {
	renderer->drawCrosshair = false;
	world->ClearAndErase();
	world->GetMainCamera()->SetCameraMode(true);
	physics->Clear();
	//add player
	auto q = Quaternion();
	playerObject = AddPlayerToWorld(Vector3(0, 5.0f, 10.0f), q);
	playerObject->SetTeamId(TEAM_RED);

	//InitGameObjects();
	//floor = AddFloorToWorld({ 0,0,0 }, { 100,1,100 });
	//InitPaintableTextureOnObject(floor);

	//map
	AddMapToWorld2();
	AddStructureToWorld();
	AddTowersToWorld();
	AddPowerUps();
	AddRespawnPoints();

#ifdef TRI_DEBUG
	AddDebugTriangleInfoToObject(floor);
#endif
}

void TutorialGame::InitOnlineGame(int teamID) {
	renderer->drawCrosshair = false;
	world->ClearAndErase();
	world->GetMainCamera()->SetCameraMode(true);
	physics->Clear();
	//add player
	auto q = Quaternion();
	playerObject = AddPlayerToWorld(Vector3(0, 5.0f, 10.0f), q);
	playerObject->SetTeamId(teamID);

	InitGameExamples();
	floor = AddFloorToWorld({ 0,0,0 }, { 100,1,100 });
	InitPaintableTextureOnObject(floor);


#ifdef TRI_DEBUG
	AddDebugTriangleInfoToObject(floor);
#endif
}

void TutorialGame::InitWorldtest2() {
	world->ClearAndErase();
	physics->Clear();
	

	InitDefaultFloorRunway();
}




void TutorialGame::RunComputeShader(GameObject* floor,int width, int height, int leftS, int rightS, int topT, int bottomT, int radius, Vector2 center,int teamID) {
	computeShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, ((OGLTexture*)floor->GetRenderObject()->GetDefaultTexture())->GetObjectID(), 0, GL_FALSE, NULL, GL_WRITE_ONLY, GL_R8UI);



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
	glUniform2i(centerLocation, (GLint)center.x, (GLint)center.y);

	int teamIDLocation = glGetUniformLocation(computeShader->GetProgramID(), "teamID");
	glUniform1i(teamIDLocation, teamID);


	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 3, "abc");
	computeShader->Execute((rightS-leftS)/8+1, (bottomT-topT)/8+1, 1);
	glPopDebugGroup();
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void TutorialGame::InitPaintableTextureOnObject(GameObject* object, bool rotated) {
	int w, h;
	if (!rotated) {
		w = (int)(object->GetTransform().GetScale().x * TEXTURE_DENSITY);
		h = (int)(object->GetTransform().GetScale().z * TEXTURE_DENSITY);
	}
	else {
		w = (int)(object->GetTransform().GetScale().x * TEXTURE_DENSITY);
		h = (int)(object->GetTransform().GetScale().z * TEXTURE_DENSITY);
	}

	object->GetRenderObject()->isPaintable = true;
	object->GetRenderObject()->maskTex = new OGLTexture();
	glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	object->GetRenderObject()->maskDimensions = { (float)w,(float)h };
//	object->GetRenderObject()->baseTex = spaceShipDiffuse;
//	object->GetRenderObject()->bumpTex = spaceShipBump;
	
//	if(object->GetRenderObject()->pbrTextures == nullptr)object->GetRenderObject()->pbrTextures = crystalPBR;
}
/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& scale, bool rotated) {
	GameObject* floor = new GameObject();

	
	AABBVolume* volume = new AABBVolume(scale);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(scale * 2)
		.SetPosition(position);

	floor->isPaintable = true;
	
	srand((int)time(0));

#ifdef OLD_PAINT
	InitPaintableTextureOnObject(floor);
	int radius = 10;
	int startIndex, numInts, leftS,rightS,topT,bottomT;
	Vector2 center;

	


	floor->ApplyPaintAtPosition(Vector3(-50, 4, 0), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center, 1);
	floor->ApplyPaintAtPosition(Vector3(50, 4, 0), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center, 1);
	floor->ApplyPaintAtPosition(Vector3(0, 4, 50), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center, 1);
	floor->ApplyPaintAtPosition(Vector3(0, 4, -50), floorSize, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
	RunComputeShader(floor, floorSize.x * 2, floorSize.z * 2, leftS, rightS, topT, bottomT, radius, center, 1);
#endif

	
	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), floorMesh, nullptr, basicShader));
	
	InitPaintableTextureOnObject(floor,rotated);
	floor->GetRenderObject()->useHeightMap = true;

	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
//	floor->GetRenderObject()->pbrTextures = grassWithWaterPBR; 

	world->AddGameObject(floor);
	
	floor->SetName("floor");
	return floor;
}

GameObject* TutorialGame::AddRunwayToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(30, 2, 30);
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
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, bool render, float inverseMass, bool physics) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	if (!render)sphere->GetRenderObject()->onlyForShadows = true;

	if (physics) {
		sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

		sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
		sphere->GetPhysicsObject()->InitSphereInertia();
	}

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddRayMarchSphereToWorld(const Vector3& position, float radius) {
	return nullptr;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, nullptr, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	InitPaintableTextureOnObject(cube);

	world->AddGameObject(cube);
	cube->SetName("cube");
	return cube;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight * 0.5f, radius * 0.5f);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform().SetScale(Vector3(radius, halfHeight, radius)).SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	InitPaintableTextureOnObject(capsule);

	world->AddGameObject(capsule);
	capsule->SetName("capsule");
	return capsule;

}

GameObject* TutorialGame::AddMonkeyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool physics) {
	GameObject* monkey = new GameObject();
	

	AABBVolume* volume = new AABBVolume(dimensions);
	monkey->SetBoundingVolume((CollisionVolume*)volume);

	monkey->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	monkey->SetRenderObject(new RenderObject(&monkey->GetTransform(), monkeyMesh, nullptr, basicShader));
	if (physics) {
		monkey->SetPhysicsObject(new PhysicsObject(&monkey->GetTransform(), monkey->GetBoundingVolume()));

		monkey->GetPhysicsObject()->SetInverseMass(inverseMass);
		monkey->GetPhysicsObject()->InitCubeInertia();
	}

	monkey->GetRenderObject()->isComplex = true;

	InitPaintableTextureOnObject(monkey);

	world->AddGameObject(monkey);

	return monkey;
}

GameObject* TutorialGame::AddWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* wall = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	wall->SetBoundingVolume((CollisionVolume*)volume);

	wall->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), basicWallMesh, wallTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(inverseMass);
	wall->GetPhysicsObject()->InitCubeInertia();

	InitPaintableTextureOnObject(wall);

	world->AddGameObject(wall);

	return wall;
}

GameObject* NCL::CSC8503::TutorialGame::AddWallToWorld2(const Vector3& position, Vector3 dimensions)
{
	GameObject* myWall = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	myWall->SetBoundingVolume((CollisionVolume*)volume);

	myWall->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	
	myWall->SetRenderObject(new RenderObject(&myWall->GetTransform(), cubeMesh, nullptr, basicShader));
	myWall->SetPhysicsObject(new PhysicsObject(&myWall->GetTransform(), myWall->GetBoundingVolume()));

	myWall->GetPhysicsObject()->SetInverseMass(0);
	myWall->GetPhysicsObject()->InitCubeInertia();

	InitPaintableTextureOnObject(myWall);
	//myWall->GetRenderObject()->pbrTextures = spaceShipPBR;

	world->AddGameObject(myWall);

	return myWall;
}

GameObject* NCL::CSC8503::TutorialGame::AddLadderToWorld(const Vector3& position, float height, bool rotated)
{
	Vector3 dimensions;
	if (rotated)
		dimensions = Vector3(1.0f, height, 0.1f);
	else
		dimensions = Vector3(0.1f, height, 1.0f);
	GameObject* ladder = new GameObject(); 
	AABBVolume* volume = new AABBVolume(dimensions);
	ladder->SetBoundingVolume((CollisionVolume*)volume);

	ladder->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	ladder->SetRenderObject(new RenderObject(&ladder->GetTransform(), cubeMesh, nullptr, basicShader));
	ladder->SetPhysicsObject(new PhysicsObject(&ladder->GetTransform(), ladder->GetBoundingVolume()));

	ladder->GetPhysicsObject()->SetInverseMass(0);
	ladder->GetPhysicsObject()->InitCubeInertia();

	ladder->SetName("ladder");

	world->AddGameObject(ladder);
	return ladder;
}

GameObject* TutorialGame::AddMaxToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* max = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	max->SetBoundingVolume((CollisionVolume*)volume);

	max->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	max->SetRenderObject(new RenderObject(&max->GetTransform(), maxMesh, nullptr, basicShader));
	max->SetPhysicsObject(new PhysicsObject(&max->GetTransform(), max->GetBoundingVolume()));

	max->GetPhysicsObject()->SetInverseMass(inverseMass);
	max->GetPhysicsObject()->InitCubeInertia();

	InitPaintableTextureOnObject(max);

	world->AddGameObject(max);

	return max;
}

GameObject* TutorialGame::AddBunnyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool physics) {
	GameObject* bunny = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	bunny->SetBoundingVolume((CollisionVolume*)volume);

	bunny->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	bunny->SetRenderObject(new RenderObject(&bunny->GetTransform(), bunnyMesh, nullptr, basicShader));
	if (physics) {
		bunny->SetPhysicsObject(new PhysicsObject(&bunny->GetTransform(), bunny->GetBoundingVolume()));

		bunny->GetPhysicsObject()->SetInverseMass(inverseMass);
		bunny->GetPhysicsObject()->InitCubeInertia();
	}
	

	InitPaintableTextureOnObject(bunny);

	world->AddGameObject(bunny);

	return bunny;
}

void TutorialGame::setLockedObjectNull() {
	lockedObject = nullptr;
}

void TutorialGame::setLockedObject(GameObject* goatPlayer) {
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
	TutorialGame::setEnemyGoat(BadGoat);
	world->AddGameObject(BadGoat);

	return BadGoat;
}

GameObject* TutorialGame::AddDebugTriangleToWorld(const Vector3& position) {
	GameObject* triangle = new GameObject();

	triangle->GetTransform().SetScale(Vector3(100, 100, 1));
	triangle->GetTransform().SetPosition(position);

	triangle->SetRenderObject(new RenderObject(&triangle->GetTransform(), triangleMesh, testCollisionTex, basicShader));
	world->AddGameObject(triangle);
	return triangle;
}

void TutorialGame::AddMapToWorld() {

	floor = AddFloorToWorld(Vector3(0, -20, 0), Vector3(250, 1, 400));
	
	//back cover walls
	walls.push_back(AddWallToWorld(Vector3(0, -22, 300), Vector3(30, 2, 1), 0.0f));      
	walls.push_back(AddWallToWorld(Vector3(0, -22, -300), Vector3(30, 2, 1), 0.0f));

	//middle cover walls
	walls.push_back(AddWallToWorld(Vector3(175, -22, 200), Vector3(20, 2, 1), 0.0f));
	walls.push_back(AddWallToWorld(Vector3(175, -22, -200), Vector3(20, 2, 1), 0.0f));
	walls.push_back(AddWallToWorld(Vector3(-175, -22, 200), Vector3(20, 2, 1), 0.0f));
	walls.push_back(AddWallToWorld(Vector3(-175, -22, -200), Vector3(20, 2, 1), 0.0f));

	//low middle cover walls
	walls.push_back(AddWallToWorld(Vector3(50, -22, 200), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(-50, -22, 200), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(50, -22, -200), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(-50, -22, -200), Vector3(15, 1, 5), 0));

	walls.push_back(AddWallToWorld(Vector3(50, -22, 100), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(-50, -22, 100), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(50, -22, -100), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(-50, -22, -100), Vector3(15, 1, 5), 0));

	walls.push_back(AddWallToWorld(Vector3(175, -22, 100), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(-175, -22, 100), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(175, -22, -100), Vector3(15, 1, 5), 0));
	walls.push_back(AddWallToWorld(Vector3(-175, -22, -100), Vector3(15, 1, 5), 0));

	//centre cover walls
	walls.push_back(AddWallToWorld(Vector3(0, -22, 10), Vector3(30, 2, 10), 0));
	walls.push_back(AddWallToWorld(Vector3(160, -22, 10), Vector3(30, 2, 10), 0));
	walls.push_back(AddWallToWorld(Vector3(-160, -22, 10), Vector3(30, 2, 10), 0));

	//dividing walls 
	walls.push_back(AddWallToWorld(Vector3(100, -25, 150), Vector3(1, 3, 200), 0));
	walls.push_back(AddWallToWorld(Vector3(-100, -25, 150), Vector3(1, 3, 200), 0));
	walls.push_back(AddWallToWorld(Vector3(100, -25, 450), Vector3(1, 3, 200), 0));
	walls.push_back(AddWallToWorld(Vector3(-100, -25, 450), Vector3(1, 3, 200), 0));

	//enclosing walls and ceiling
	AddFloorToWorld(Vector3(0, 100, 0), Vector3(250, 1, 400));

	//side walls
	walls.push_back(AddWallToWorld(Vector3(250, -50, 1200), Vector3(4, 20, 800), 0));
	walls.push_back(AddWallToWorld(Vector3(-250, -50, 1200), Vector3(4, 20, 800), 0));

	//back walls
	walls.push_back(AddWallToWorld(Vector3(0, -50, 405), Vector3(250, 20, 4), 0));
	walls.push_back(AddWallToWorld(Vector3(0, -50, -395), Vector3(250, 20, 4), 0));

	for (GameObject*& wall : walls) {
		InitPaintableTextureOnObject(wall);
	}

}

void NCL::CSC8503::TutorialGame::AddMapToWorld2()
{
	//floor and enclosing walls 
	AddFloorToWorld({0, 0, 0}, {100, 1, 200});

	//visible walls
	walls.push_back(AddWallToWorld2({ 100, 5, 0 }, { 1, 5, 200 }));
	walls.push_back(AddWallToWorld2({ -100, 5, 0 }, { 1, 5, 200 }));

	walls.push_back(AddWallToWorld2({ 0, 5, 200 }, { 100, 5, 1 }));
	walls.push_back(AddWallToWorld2({ 0, 5, -200 }, { 100, 5, 1 }));

	//invisible walls
	AddWallToWorld2({ 100, 5, 0 }, { 1, 50, 200 })->SetRenderObject(nullptr);
	AddWallToWorld2({ -100, 5, 0 }, { 1, 50, 200 })->SetRenderObject(nullptr);

	AddWallToWorld2({ 0, 5, 200 }, { 100, 50, 1 })->SetRenderObject(nullptr);
	AddWallToWorld2({ 0, 5, -200 }, { 100, 50, 1 })->SetRenderObject(nullptr);

	//dividing walls
	/*walls.push_back(AddWallToWorld2({ -50, 7, 0 }, { 1, 7, 100 }));
	walls.push_back(AddWallToWorld2({ 50, 7, 0 }, { 1, 7, 100 }));*/

	walls.push_back(AddWallToWorld2({ -50, 7, 50 }, { 1, 7, 40 }));
	walls.push_back(AddWallToWorld2({ -50, 7, -50 }, { 1, 7, 40 }));

	walls.push_back(AddWallToWorld2({ 50, 7, 50 }, { 1, 7, 40 }));
	walls.push_back(AddWallToWorld2({ 50, 7, -50 }, { 1, 7, 40 }));

	//back cover walls
	walls.push_back(AddWallToWorld2({ 0, 5, 125 }, { 25, 5, 1 }));
	walls.push_back(AddWallToWorld2({ 0, 5, -125 }, { 25, 5, 1 }));

	//middle low cover walls
	walls.push_back(AddWallToWorld2({ 30, 2.5, 75 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ -30, 2.5, 75 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ 0, 2.5, 75 }, { 10, 2.5, 1 }));

	walls.push_back(AddWallToWorld2({ -15, 2.5, 35 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ 15, 2.5, 35 }, { 10, 2.5, 1 }));

	walls.push_back(AddWallToWorld2({ 30, 2.5, -75 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ -30, 2.5, -75 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ 0, 2.5, -75 }, { 10, 2.5, 1 }));

	walls.push_back(AddWallToWorld2({ -15, 2.5, -35 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ 15, 2.5, -35 }, { 10, 2.5, 1 }));

	//side low cover walls 
	walls.push_back(AddWallToWorld2({ 75, 2.5, 50 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ -75, 2.5, 50 }, { 10, 2.5, 1 }));

	walls.push_back(AddWallToWorld2({ 75, 2.5, -50 }, { 10, 2.5, 1 }));
	walls.push_back(AddWallToWorld2({ -75, 2.5, -50 }, { 10, 2.5, 1 }));

	//AddPowerUps();
}

void NCL::CSC8503::TutorialGame::AddStructureToWorld()
{
	//middle structure
	//pillars
	AddCubeToWorld({12, 7, 8}, {1, 7, 1}, 0.0f);
	AddCubeToWorld({ -12, 7, 8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ 12, 7, -8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ -12, 7, -8 }, { 1, 7, 1 }, 0.0f);

	//platform 
	AddCubeToWorld({ 0, 13, 0 }, { 13, 1, 9 }, 0.0f);

	//side structures
	//pillars
	AddCubeToWorld({ 87, 7, 8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ 63, 7, 8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ 87, 7, -8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ 63, 7, -8 }, { 1, 7, 1 }, 0.0f);

	//platform 
	AddCubeToWorld({ 75, 13, 0 }, { 13, 1, 9 }, 0.0f);
	AddLadderToWorld({88.5, 7, 0}, 7.0f, false);

	//pillars
	AddCubeToWorld({ -87, 7, 8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ -63, 7, 8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ -87, 7, -8 }, { 1, 7, 1 }, 0.0f);
	AddCubeToWorld({ -63, 7, -8 }, { 1, 7, 1 }, 0.0f);

	//platform 
	AddCubeToWorld({ -75, 13, 0 }, { 13, 1, 9 }, 0.0f);
	AddLadderToWorld({ -88.5, 7, 0 }, 7.0f, false);

	//paths betweeen platforms
	AddCubeToWorld({38, 13, 0}, {25, 1, 6}, 0.0f);
	AddCubeToWorld({ -38, 13, 0 }, { 25, 1, 6 }, 0.0f);

	//walls on platforms
	AddCubeToWorld({ 0, 15.5, 8 }, { 13, 1.5, 1 }, 0.0f);
	AddCubeToWorld({ 0, 15.5, -8 }, { 13, 1.5, 1 }, 0.0f);

	AddCubeToWorld({ -75, 15.5, 8 }, { 13, 1.5, 1 }, 0.0f);
	AddCubeToWorld({ -75, 15.5, -8 }, { 13, 1.5, 1 }, 0.0f);

	AddCubeToWorld({ 75, 15.5, 8 }, { 13, 1.5, 1 }, 0.0f);
	AddCubeToWorld({ 75, 15.5, -8 }, { 13, 1.5, 1 }, 0.0f);
}

void NCL::CSC8503::TutorialGame::AddTowersToWorld()
{
/*	AddCubeToWorld({70, 12.5, 150}, {2.5 , 12.5, 2.5}, 0.0f)->GetRenderObject()->pbrTextures = rockPBR;
	AddLadderToWorld({ 70, 12.5, 153 }, 12.5f, true)->GetRenderObject()->pbrTextures = rockPBR;

	AddCubeToWorld({ -70, 12.5, -150 }, { 2.5 , 12.5, 2.5 }, 0.0f)->GetRenderObject()->pbrTextures = rockPBR;
	AddLadderToWorld({ -70, 12.5, -153 }, 12.5f, true)->GetRenderObject()->pbrTextures = rockPBR;*/
}

void NCL::CSC8503::TutorialGame::AddPlatformsToWorld()
{
}

void NCL::CSC8503::TutorialGame::AddPowerUps()
{

//	propSystem->SpawnWeaponUp({ 0, 16, 0 }, grassWithWaterPBR);

	propSystem->SpawnHeal({ 0, 5, 150 });
	propSystem->SpawnHeal({ 0, 5, -150 });
	propSystem->SpawnHeal({ 70, 5, 100 });
	propSystem->SpawnHeal({ -70, 5, -100 });
	propSystem->SpawnHeal({ -70, 5, 100 });
	propSystem->SpawnHeal({ 70, 5, -100 });

	propSystem->SpawnSpeedUp({ 75, 5, 0 });
	propSystem->SpawnSpeedUp({ -75, 5, 0 });

	propSystem->SpawnShield({ 0, 5, 50 });
	propSystem->SpawnShield({ 0, 5, -50 });
}

void NCL::CSC8503::TutorialGame::AddRespawnPoints()
{
	//back respawn points
	RespawnPoint* respawnPoint = new RespawnPoint({ 0, 5, 150 });
	respawnPoint->AddRespawnPoint(respawnPoint);
	RespawnPoint* rp = new RespawnPoint({ 0, 5, -150 });
	respawnPoint->AddRespawnPoint(rp);

	//side respawn points
	rp = new RespawnPoint({ 70, 5, 0 });
	respawnPoint->AddRespawnPoint(rp);
	rp = new RespawnPoint({ -70, 5, 0 });
	respawnPoint->AddRespawnPoint(rp);

	//corner respawn points 
	rp = new RespawnPoint({ 70, 5, -150 });
	respawnPoint->AddRespawnPoint(rp);
	rp = new RespawnPoint({ -70, 5, -150 });
	respawnPoint->AddRespawnPoint(rp);
	rp = new RespawnPoint({ 70, 5, 150 });
	respawnPoint->AddRespawnPoint(rp);
	rp = new RespawnPoint({ -70, 5, 150 });
	respawnPoint->AddRespawnPoint(rp);
}

playerTracking* TutorialGame::AddPlayerToWorld(const Vector3& position, Quaternion & orientation, int team , RespawnPoint* rp) {
	float meshSize = 2.0f;
	float inverseMass = 0.3f;

	playerTracking* character = new playerTracking(world);
	AABBVolume* volume = new AABBVolume(Vector3{ 2,2,2 });

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position).SetOrientation(orientation);
	//character->GetTransform().setGoatID(7);
	character->setImpactAbsorbtionAmount(0.9f);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), playerMesh, nullptr, characterShader ));
	character->GetRenderObject()->isAnimated = true;

	for (unsigned int i = 0; i < playerMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = playerMaterial->GetMaterialForLayer(i);
		const std::string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		std::string path = *filename;
		character->GetRenderObject()->matTextures.emplace_back(renderer->LoadTexture(path));
	}
	character->GetRenderObject()->anim = character->GetCurrentAnimation();
	animatedObjects.emplace_back(character->GetRenderObject());

	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->setCoeficient(0.55f);
	character->GetPhysicsObject()->SetElasticity(0.0f);
	InitPaintableTextureOnObject(character);

	world->AddGameObject(character);
	character->SetName("character");
	character->SetTeamId(1);
	return character;
}


Projectile* TutorialGame::AddBulletToWorld(playerTracking* playableCharacter) {
	if (playableCharacter->getBulletVectorSize() <= 200) {
		return useNewBullet(playableCharacter);
	}

	return nullptr;
}


Projectile* TutorialGame::useNewBullet(playerTracking* passedPlayableCharacter) {
	//gun wepType = passedPlayableCharacter->getWeponType();

	//Projectile* sphere = objectpool->GetObject2();
	//float bulletsInverseMass = sphere->getWeight();
	//float radius = sphere->getProjectileRadius();
	////Vector3 playerDirectionVector = (Vector3::Cross((passedPlayableCharacter->GetTransform().GetOrientation().ToEuler()) , Vector3 {1,0,0})).Normalised();
	//Vector3 playerDirectionVector = (passedPlayableCharacter->GetTransform().GetOrientation() * Vector3 { 0, 0, -1 });
	//Vector3 sphereSize = { radius,radius,radius };
	//Vector3 position = passedPlayableCharacter->GetTransform().GetPosition();
	//SphereVolume* volume = new SphereVolume(radius);
	//sphere->setBulletDirectionVector(playerDirectionVector);
	//sphere->SetBoundingVolume((CollisionVolume*)volume);
	//sphere->GetTransform().SetScale(sphereSize);
	//sphere->GetTransform().SetPosition(position - Vector3{ 0,0,10 });
	//sphere->GetTransform().SetPosition((position)-(Vector3{ 0,0,10 }));

	//sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	//sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));
	//sphere->GetRenderObject()->SetColour(passedPlayableCharacter->getPaintColor());
	//PhysicsObject* physicsBullet = sphere->GetPhysicsObject();
	//if (!sphere->ProjectileAffectedByGravity() || true) {
	//	physicsBullet->SetAffectedByGravityFalse();
	//}
	//sphere->GetPhysicsObject()->SetInverseMass(bulletsInverseMass);
	//sphere->GetPhysicsObject()->InitSphereInertia();

	//world->AddGameObject(sphere);
	return nullptr;

}


Projectile* TutorialGame::FireBullet(playerTracking* selectedPlayerCharacter) {
	Projectile* loadedBullet = useNewBullet(selectedPlayerCharacter);
	//selectedPlayerCharacter->addToBulletsUsed(loadedBullet);
	PhysicsObject* physicsShot = loadedBullet->GetPhysicsObject();
	//physicsShot->SetLayerID(); // set Id so bullets cannot collied with each other and players.
	//loadedBullet->GetTransform().setDestructable();
	physicsShot->SetLinearVelocity({ 0,0,0 });
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



//void TutorialGame::setGoatCharacter(playerTracking* assignCharcter) {
//	goatCharacter = assignCharcter;
//}

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
		/*playerState *a = new playerState();
		a->setCurrentPlayerAction(a->jumping);
		cout<< "current player action = " << a->getCurrentPlayerAction()<< endl;*/
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
	float inverseMass	= 0.5f;

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



void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0), {100,1,100});
}

void TutorialGame::InitDefaultFloorRunway() {
	AddRunwayToWorld(Vector3(0, -20, 0));
}

void TutorialGame::InitGameExamples() {
	AddCubeToWorld(Vector3(0.0f, 5.0f, 0.0f), Vector3(2.5f, 2.5f, 2.5f), 0.5f);
	AddCapsuleToWorld(Vector3(0.0f, 5.0f, 5.0f), 2.5, 2.5);
	
	//TODO
	auto q = Quaternion();
	playerObject = AddPlayerToWorld(Vector3(0, 5.0f, 10.0f), q);
	lockedObject = playerObject; 
	//TestCode of Item
	/*Item* p;
	propSystem->SpawnItem(Vector3(6, 3, 6));
	p = propSystem->SpawnSpeedUp(Vector3(9, 3, 6));
	world->AddGameObject(p);
	p = propSystem->SpawnShield(Vector3(12, 3, 6));
	world->AddGameObject(p);
	p = propSystem->SpawnHeal(Vector3(15, 3, 6));
	world->AddGameObject(p);
	p = propSystem->SpawnWeaponUp(Vector3(18, 3, 6));
	world->AddGameObject(p);*/

	

	//AddPlayerToWorld(Vector3(0, 5.0f, 10.0f), q);
	//AddEnemyToWorld(Vector3(5, 5, 0));
	//AddBonusToWorld(Vector3(10, 5, 0));
	AddPowerUps();
	AddRespawnPoints(); 
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0), { 100,1,100 });
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
				AddSphereToWorld(position, sphereRadius,true);
			}
		}
	}
}

void TutorialGame::InitMixedGridWorldtest(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);
	//	chainBallTest();
	StateGameObject* AddStateObjectToWorld(const Vector3 & position);
	Quaternion q;
	playerTracking* player1 = AddPlayerToWorld(Vector3(-10, -10, 0),q);
	movePlayer(player1);
	setLockedObject(player1);
	Vector3 position1 = Vector3(0 * colSpacing, 10.0f, 0 * rowSpacing);
	Vector3 position2 = Vector3(1 * colSpacing, 10.0f, 1 * -rowSpacing);

	AddCubeToWorld(position1, cubeDims);

	//AddSphereToWorld(position2, sphereRadius);

}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
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


void TutorialGame::DispatchComputeShaderForEachTriangle(GameObject* object, Vector3 spherePosition, float sphereRadius, int teamID, bool clearMask) {

	Matrix4 modelMatrix = object->GetTransform().GetMatrix();
	
	triComputeShader->Bind();

	

	OGLMesh* mesh = (OGLMesh*)object->GetRenderObject()->GetMesh();
	unsigned int numTris = mesh->GetIndexCount() / 3;
	GLuint vao = mesh->vao;
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER,tempSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mesh->attributeBuffers[VertexAttribute::Positions]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mesh->attributeBuffers[VertexAttribute::TextureCoords]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mesh->indexBuffer);

	Vector2 maskDims = object->GetRenderObject()->maskDimensions;
	
	
	if(clearMask)glClearTexImage((((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID()), 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
	
	
#ifdef TRI_DEBUG
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, ((OGLTexture*)(object->GetRenderObject()->triDataTex))->GetObjectID(), 0, GL_FALSE, NULL, GL_READ_WRITE, GL_RGBA16);
#endif

	glActiveTexture(GL_TEXTURE1);
	glBindImageTexture(1, ((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID(), 0, GL_FALSE, NULL, GL_READ_WRITE, GL_R8UI);

	int radiusLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "sphereRadius");
	int centerLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "sphereCenter");
	int textureWidthLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "textureWidth");
	int textureHeightLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "textureHeight");
	int isComplexLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "isComplex");
	int modelMatrixLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "modelMatrix");
	int numTrisLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "numTris");
	int teamIDLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "teamID");
	int newMethodLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "newMethod");
	glUniform1f(radiusLocation, sphereRadius);
	glUniform3fv(centerLocation,1, spherePosition.array);
	glUniform1i(textureWidthLocation, (GLint)object->GetRenderObject()->maskDimensions.x);
	glUniform1i(textureHeightLocation, (GLint)object->GetRenderObject()->maskDimensions.y);
	glUniform1i(isComplexLocation, object->GetRenderObject()->isComplex);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (float*)&modelMatrix);
	glUniform1i(numTrisLocation, numTris);
	glUniform1i(teamIDLocation, teamID);
	glUniform1i(newMethodLocation, renderer->newMethod);
	
	
	

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, debugTriangleSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, debugTriangleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	int zero = 0;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleBoolSSBO);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_INT, &zero);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, triangleBoolSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleRasteriseSSBO);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_INT, &zero);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, triangleRasteriseSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//int* ints = new int[highestTriCount];
	
	triComputeShader->Execute((numTris)/64+1, 1, 1);//todo change number of thread groups
	int* ints = (int*)glMapNamedBuffer(triangleBoolSSBO, GL_READ_ONLY);//todo store 32 bools in one int
	unsigned int* ints2 = (unsigned int*)glMapNamedBuffer(triangleRasteriseSSBO, GL_READ_ONLY);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, highestTriCount * sizeof(int), ints);
	unsigned int numTrisHit = 0;
	unsigned int maxWidth = 0;
	unsigned int maxHeight = 0;


	std::vector<std::tuple<unsigned int, unsigned int>> coords;
	std::vector<int> indices;
#pragma region 2 ints in 1
	/*unsigned int rightHalf = 0;
	for (int i = 0; i < sizeof(unsigned int) / 2; i++)
	{
		rightHalf += 1 << i;
	}
	unsigned int leftHalf = 0;
	for (int i = sizeof(unsigned int) / 2; i < sizeof(unsigned int); i++)
	{
		leftHalf += 1 << i;
	}*/
#pragma endregion
	for (int i = 0; i < highestTriCount; i++)
	{
		if (ints[i]) {
			numTrisHit++;
			indices.push_back(i);
			if(ints2[4 * i + 0] > maxWidth) maxWidth = ints2[4 * i + 0];
			if(ints2[4 * i + 1] > maxHeight) maxHeight = ints2[4 * i + 1];
			coords.push_back({ ints2[4 * i + 2], ints2[4 * i + 3] });
		}
	}

	glUnmapNamedBuffer(triangleBoolSSBO);
	glUnmapNamedBuffer(triangleRasteriseSSBO);



	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	triComputeShader->Unbind();

	triRasteriseShader->Bind();

	uint32_t* data = new uint32_t[numTrisHit * 3];
	for (int i = 0; i < (int)numTrisHit; i++)
	{
		data[3 * i + 0] = indices[i];
		data[3 * i + 1] = std::get<0>(coords[i]);
		data[3 * i + 2] = std::get<1>(coords[i]);
	}
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleRasteriseSSBOSecondShader);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, triangleRasteriseSSBOSecondShader);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER,0, sizeof(uint32_t) * numTrisHit * 3, data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	radiusLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "sphereRadius");
	centerLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "sphereCenter");
	textureWidthLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "textureWidth");
	textureHeightLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "textureHeight");
	teamIDLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "teamID");
	modelMatrixLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "modelMatrix");
	int chunkWidthLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "chunkWidth");
	int chunkHeightLocation = glGetUniformLocation(triRasteriseShader->GetProgramID(), "chunkHeight");

	glUniform1f(radiusLocation, sphereRadius);
	glUniform3fv(centerLocation, 1, spherePosition.array);
	glUniform1i(textureWidthLocation, (GLint)object->GetRenderObject()->maskDimensions.x);
	glUniform1i(textureHeightLocation, (GLint)object->GetRenderObject()->maskDimensions.y);
	glUniform1i(teamIDLocation, teamID);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (float*)&modelMatrix);
	glUniform1i(chunkWidthLocation, maxWidth);
	glUniform1i(chunkHeightLocation, maxHeight);

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 6, "second");
	triRasteriseShader->Execute(maxWidth/8+1,maxHeight/8+1,numTrisHit);
	glPopDebugGroup();
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	triRasteriseShader->Unbind();
	delete[] data;
	
}

void TutorialGame::SetUpTriangleSSBOAndDataTexture()
{
	
	//todo delete this, just for testing
	std::array<char, MAX_TRIS> noise;
	for (int i = 0; i < MAX_TRIS; i++)
	{
		noise[i] = (char)rand() / (char)RAND_MAX;
	}
	
	glGenBuffers(1, &(triangleSSBO));
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_TRIS * sizeof(float) * 15, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, triangleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &(debugTriangleSSBO));
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, debugTriangleSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_TRIS * sizeof(float) * 15, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, debugTriangleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	

	/*glGenBuffers(1, &triangleBoolSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleBoolSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_TRIS * sizeof(bool), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, MAX_TRIS * sizeof(bool), nullptr);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, triangleBoolSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/
	
}

void TutorialGame::AddDebugTriangleInfoToObject(GameObject* object) {
	object->GetRenderObject()->triDataTex = new OGLTexture();
	glBindTexture(GL_TEXTURE_1D, ((OGLTexture*)(object->GetRenderObject()->triDataTex))->GetObjectID());
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, MAX_TRIS, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_1D, 0);

	
}

bool TutorialGame::SphereTriangleIntersection(Vector3 sphereCenter, float sphereRadius, Vector3 v0, Vector3 v1, Vector3 v2, Vector3& intersectionPoint) {
	Vector3 triangleNormal = (Vector3::Cross(v1-v0,v2-v0)).Normalised();

	// Compute the distance of the sphere center to the triangle plane
	float distance = Vector3::Dot(triangleNormal, sphereCenter - v0);

	// Check if the sphere is behind or too far away from the triangle
	if (distance > sphereRadius || distance < -sphereRadius)
		return false;

	// Compute the projection of the sphere center onto the triangle plane
	Vector3 projection = sphereCenter - triangleNormal * distance;

	// Check if the projection is inside the triangle
	Vector3 edge0 = v1 - v0;
	Vector3 vp0 = projection - v0;
	if (Vector3::Dot(triangleNormal, Vector3::Cross(edge0,vp0)) < 0)
		return false;

	Vector3 edge1 = v2 - v1;
	Vector3 vp1 = projection - v1;
	if (Vector3::Dot(triangleNormal, Vector3::Cross(edge1, vp1)) < 0)
		return false;

	Vector3 edge2 = v0 - v2;
	Vector3 vp2 = projection - v2;
	if (Vector3::Dot(triangleNormal, Vector3::Cross(edge2, vp2)) < 0)
		return false;

	// Compute the intersection point
	float t = sphereRadius * sphereRadius - ((sphereCenter - projection).Length()) *
		((sphereCenter - projection).Length());

	if (t < 0)
		return false;

	//intersectionPoint = projection - triangleNormal * sqrt(t);
	intersectionPoint = projection;
	
	return true;
}

void TutorialGame::UpdateAnimations(float dt) {

	for (auto& i : animatedObjects) {
		const Matrix4* invBindPose = i->GetMesh()->GetInverseBindPose().data();

		const Matrix4* frameData = i->anim->GetJointData(i->currentFrame);

		i->frameMatrices.clear();
		for (unsigned int x = 0; x < i->GetMesh()->GetJointCount(); ++x) {
			i->frameMatrices.emplace_back(frameData[x] * invBindPose[x]);
		}
		i->frameTime -= dt;
		while (i->frameTime < 0.0f) {
			i->currentFrame = (i->currentFrame + 1) % i->anim->GetFrameCount();
			i->frameTime += 1.0f / i->anim->GetFrameRate();
		}

	}

}