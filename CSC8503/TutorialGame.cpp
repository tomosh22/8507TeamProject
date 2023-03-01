#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include <minmax.h>
#include<cmath>

#include "playerTracking.h"
#include"Projectile.h"

#include<iostream>

using namespace std;
using namespace NCL;
using namespace CSC8503;

#define TRI_DEBUG
//#define OLD_PAINT

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
	testStateObject = nullptr;

	objectpool = new ObjectPool<Projectile>(100);

	InitialiseAssets();

	SetUpTriangleSSBOAndDataTexture();

	Vector3 triA(2, 2, 1);
	Vector3 triB(7, 4, 1);
	Vector3 triC(5, 9, 1);
	Vector3 point(3, 3, 1);
	Vector3 uvw = PhysicsObject::WorldSpaceToBarycentricCoords(point,triA,triB,triC);
	std::cout << uvw;


	
	
	//this was me
	maxRayMarchSpheres = 100;
	glGenBuffers(1, &rayMarchSphereSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rayMarchSphereSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxRayMarchSpheres * sizeof(RayMarchSphere), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, rayMarchSphereSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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
	renderer->imguiptrs.testSphereCenter = &testSphereCenter;
	renderer->imguiptrs.testSphereRadius = &testSphereRadius;
	for (int i = 0; i < 1000 * 1000; i++)
	{
		zeros[i] = 0;
	}

	return;

	
}



void TutorialGame::InitQuadTexture() {
	int width = (renderer->GetWindowWidth());
	int height = (renderer->GetWindowHeight());
	//std::array<float, 1280 * 720 * 4>* data = new std::array<float, 1280 * 720 * 4>();//todo dont hardcode
	quadTex = new OGLTexture();
	renderer->quad = new RenderObject(nullptr, OGLMesh::GenerateQuadWithIndices(), quadTex, quadShader);
	//for (int i = 0; i < 1280*720*4; i++)
	//{ 
	//	data->at(i) = (float)rand() / (float)RAND_MAX;
	//}
	
	glBindTexture(GL_TEXTURE_2D, (((OGLTexture*)renderer->quad->GetDefaultTexture())->GetObjectID()));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

	//todo maybe move this somewhere else? still somewhat related
	maxSteps = 50;
	hitDistance = 0.1;
	noHitDistance = 1000;
	debugValue = 1;
	rayMarchDepthTest = true;
	renderer->imguiptrs.rayMarchMaxSteps = &maxSteps;
	renderer->imguiptrs.rayMarchHitDistance = &hitDistance;
	renderer->imguiptrs.rayMarchNoHitDistance = &noHitDistance;
	renderer->imguiptrs.debugValue = &debugValue;
	renderer->imguiptrs.depthTest = &rayMarchDepthTest;
}

//raymarching
void TutorialGame::DispatchComputeShaderForEachPixel() {
	int width = renderer->GetWindowWidth();
	int height = renderer->GetWindowHeight();

	rayMarchComputeShader->Bind();

	float screenAspect = (float)width / (float)height;
	//world->GetMainCamera()->SetFieldOfVision(90);

	Matrix4 viewMatrix = world->GetMainCamera()->BuildViewMatrix();
	//std::cout << viewMatrix << '\n';
	Matrix4 projMatrix = world->GetMainCamera()->BuildProjectionMatrix(screenAspect);
	Vector3 cameraPos = world->GetMainCamera()->GetPosition();
	
	std::vector<float> buffer;
	buffer.resize(width * height);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer.data());
	glBindTexture(GL_TEXTURE_2D, depthBufferTex);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_DEPTH_COMPONENT,
		renderer->GetWindowWidth(),
		renderer->GetWindowHeight(),
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		buffer.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	

	int projLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "viewMatrix");
	int cameraPosLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "cameraPos");
	int maxStepsLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "maxSteps");
	int hitDistanceLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "hitDistance");
	int noHitDistanceLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "noHitDistance");
	int viewportWidthLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "viewportWidth");
	int viewportHeightLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "viewportHeight");
	int numSpheresLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "numSpheres");
	int depthTexLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "depthTex");
	int nearPlaneLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "nearPlane");
	int farPlaneLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "farPlane");
	int debugValueLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "debugValue");
	int depthTestValueLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "depthTest");


	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);
	glUniform3fv(cameraPosLocation, 1, cameraPos.array);
	glUniform1i(maxStepsLocation, maxSteps);
	glUniform1f(hitDistanceLocation, hitDistance);
	glUniform1f(noHitDistanceLocation, noHitDistance);
	glUniform1i(viewportWidthLocation, width);
	glUniform1i(viewportHeightLocation, height);
	glUniform1i(numSpheresLocation, rayMarchSpheres.size()+2);//one for debug sphere
	glUniform1f(nearPlaneLocation, world->GetMainCamera()->GetNearPlane());
	glUniform1f(farPlaneLocation, world->GetMainCamera()->GetFarPlane());
	glUniform1f(debugValueLocation, debugValue);
	glUniform1i(depthTestValueLocation, rayMarchDepthTest);


	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, (((OGLTexture*)renderer->quad->GetDefaultTexture())->GetObjectID()), 0, GL_FALSE, NULL, GL_WRITE_ONLY, GL_RGBA16F);



	glUniform1i(depthTexLocation, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthBufferTex);

	rayMarchComputeShader->Execute(width/8+1, height/8+1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

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
	//this was me
	triangleMesh = OGLMesh::GenerateTriangleWithIndices();
	monkeyMesh = renderer->LoadMesh("monkey.msh");
	floorMesh = renderer->LoadMesh("Corridor_Floor_Basic.msh");
	maxMesh = renderer->LoadMesh("Rig_Maximilian.msh");
	basicWallMesh = renderer->LoadMesh("corridor_Wall_Straight_Mid_end_L.msh");
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
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
	metalTex = renderer->LoadTexture("metal.png");
	testBumpTex = renderer->LoadTexture("testBump.jpg");

	//this was me
	computeShader = new OGLComputeShader("compute.glsl");
	quadShader = new OGLShader("quad.vert", "quad.frag");

	triComputeShader = new OGLComputeShader("tris.comp");

	rayMarchComputeShader = new OGLComputeShader("rayMarchCompute.glsl");

	
	InitQuadTexture();
	
	InitCamera();
	InitWorld();

	/*AddFloorToWorld({ 0, 0, 0 });
	AddPlayerToWorld({ 0, 1, 0 });*/
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

	delete objectpool;

	//todo delete texture array
	//todo delete compute shader
}



void TutorialGame::UpdateGame(float dt) {
	if (GAME_MODE_DEFAULT == gameMode) {
		SelectMode();
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
		return;
	}
	if (GAME_MODE_GRAPHIC_TEST == gameMode) {
		//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 4, "cube");
		//DispatchComputeShaderForEachTriangle(testCube);
		//glPopDebugGroup();
		//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 6, "monkey");
		//DispatchComputeShaderForEachTriangle(monkey);
		//glPopDebugGroup();
		//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "floor");
		//DispatchComputeShaderForEachTriangle(floor);
		//glPopDebugGroup();
		//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "walls");
		//for (GameObject*& wall : walls) {
			//DispatchComputeShaderForEachTriangle(wall);
		//}
		//glPopDebugGroup();
	}
	
	timePassed += dt;
	//TODO DELETE THIS !!!
	DispatchComputeShaderForEachPixel();
	if(worldFloor != nullptr)
	Debug::DrawAxisLines(worldFloor->GetTransform().GetMatrix());

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);
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

	//Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	SelectObject();
	MoveSelectedObject();
	//movePlayer(goatCharacter);

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);

	/*float testFloat = float(1000) / float(55);
	if (1000 * dt > testFloat)std::cout << "fps drop\n";*/

	//timePassed = 0;

	if (GAME_MODE_GRAPHIC_TEST == gameMode) {
    UpdateRayMarchSpheres();
		SendRayMarchData();
	}
}

void TutorialGame::SelectMode() {
	string text = "1. Graphic Test Mode.";
	Debug::Print(text, Vector2(35, 30), Debug::GREEN);
	text = "2. Physical Test Mode.";
	Debug::Print(text, Vector2(35, 50), Debug::GREEN);
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM1)) {
		gameMode = GAME_MODE_GRAPHIC_TEST;
		InitGraphicTest();
	}
	else if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM2)) {
		gameMode = GAME_MODE_PHISICAL_TEST;
		InitPhysicalTest();
	}
}

void TutorialGame::UpdateRayMarchSpheres() {
	
	std::map<unsigned int, std::function<Vector3(float)>> sinFunctions{
		{0, [](float timePassed) {return Vector3(std::sin(timePassed) * 20,0,0); }},
		{1, [](float timePassed) {return Vector3(std::sin(timePassed) * -20,0,0); }},
		{2, [](float timePassed) {return Vector3(0, std::sin(timePassed) * 20, 0); }},
		{3, [](float timePassed) {return Vector3(0, std::sin(timePassed) * -20, 0); }},
		{4, [](float timePassed) {return Vector3(0, 0, std::sin(timePassed) * 20); }},
		{5, [](float timePassed) {return Vector3(0, 0, std::sin(timePassed) * -20); }}
	};
	std::map<unsigned int, Vector3> colours{
		{0, {1,0,0}},
		{1, {0,1,0}},
		{2, {0,0,1}},

		{3, {0,1,1}},
		{4, {1,0,1}},
		{5, {1,1,0}},
	};
	unsigned int count = 0;
	for (RayMarchSphere* sphere : rayMarchSpheres)
	{
		std::function<Vector3(float)> sinFunction = sinFunctions.at(count % 6);
		sphere->GetTransform().SetPosition(sinFunction(timePassed) + Vector3(20,20,20));
		sphere->center = sphere->GetTransform().GetPosition();
		Vector3 scale = sphere->GetTransform().GetScale();
		sphere->radius = scale.x;
		sphere->color = colours.at(count %6);
		count++;
	}
}

void TutorialGame::SendRayMarchData() {
	//just for testing, i know this is a horrible way of doing this
	int numSpheresSent = 0;
	unsigned int size = sizeof(RayMarchSphere);
	for (RayMarchSphere* sphere : rayMarchSpheres)
	{
		
		int offset = numSpheresSent * sizeof(float) * 7;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, rayMarchSphereSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(float), &(sphere->center.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + sizeof(float), sizeof(float), &(sphere->center.y));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 2 * sizeof(float), sizeof(float), &(sphere->center.z));

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 3 * sizeof(float), sizeof(float), &(sphere->radius));

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 4 * sizeof(float), sizeof(float), &(sphere->color.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 5 * sizeof(float), sizeof(float), &(sphere->color.y));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 6 * sizeof(float), sizeof(float), &(sphere->color.z));

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		numSpheresSent++;
	}

	return;
	Vector3 position = testSphereCenter;
	float radius = testSphereRadius;
	int offset = numSpheresSent * sizeof(RayMarchSphere);
	Vector3 color = { 1,1,0 };
	float radiusExtension = radius / 2;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rayMarchSphereSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(float), &(position.x));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + sizeof(float), sizeof(float), &(position.y));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 2 * sizeof(float), sizeof(float), &(position.z));

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 3 * sizeof(float), sizeof(float), &(radius));

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 4 * sizeof(float), sizeof(float), &(color.x));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 5 * sizeof(float), sizeof(float), &(color.y));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 6 * sizeof(float), sizeof(float), &(color.z));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
	/*if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		for (int i = 0; i < 1; i++)
		{
			Vector3 halfDims = worldFloor->GetTransform().GetScale() / 2;
			float randX = (rand() % 200) - 100;
			float randY = (rand() % 200) - 100;
			Vector3 randVec(randX, 2, randY);
			Vector2 center;
			int radius = 10;
			int startIndex, numInts, leftS, rightS, topT, bottomT;

			//worldFloor->ApplyPaintAtPosition(randVec, halfDims, radius, startIndex, numInts, leftS, rightS, topT, bottomT, center);
			//RunComputeShader(worldFloor,200,200, leftS, rightS, topT, bottomT, radius, center);


		}
		
	}*/
  if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		float randX = (rand() % 200) - 100;
		float randZ = (rand() % 200) - 100;
		Vector3 randVec(randX, 2, randZ);
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "floor");
		DispatchComputeShaderForEachTriangle(floor, {randX,5,randZ},10);
		glPopDebugGroup();
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

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
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

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
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
	gameMode = GAME_MODE_DEFAULT;
}

void TutorialGame::InitGraphicTest() {
	world->ClearAndErase();
	physics->Clear();

	//InitDefaultFloor();

	//position is irrelevant at this point in testing as im overriding position later
	//for raymarching
	for (int i = 0; i < 10; i++)
	{
		AddSphereToWorld({ 0,0,0 }, 10, false);
	}
	

	

	//testCube = AddCubeToWorld(Vector3(), Vector3(100, 100, 100));
	floor = AddFloorToWorld({ 0,0,0 }, { 100,1,100 });
	floor->SetName(std::string("floor"));

	//testTriangle = AddDebugTriangleToWorld({ 0,200,0 });

	monkey = AddMonkeyToWorld({ 0,20,100 }, { 5,5,5 });
	monkey->SetName(std::string("monkey"));
	monkey->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, 180, 0));
	walls.push_back(AddFloorToWorld({ 0,25,-20 }, {100,1,25},true));
	walls.back()->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(90, 0, 0));
	walls.back()->SetName(std::string("back"));

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
	return;
}

void TutorialGame::InitPhysicalTest() {
	InitGameExamples();
	InitDefaultFloor();
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
	glUniform2i(centerLocation, center.x,center.y);

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
		w = object->GetTransform().GetScale().x * TEXTURE_DENSITY;
		h = object->GetTransform().GetScale().z * TEXTURE_DENSITY;
	}
	else {
		w = object->GetTransform().GetScale().x * TEXTURE_DENSITY;
		h = object->GetTransform().GetScale().z * TEXTURE_DENSITY;
	}

	object->GetRenderObject()->isPaintable = true;
	object->GetRenderObject()->maskTex = new OGLTexture();
	glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	object->GetRenderObject()->maskDimensions = { (float)w,(float)h };
	object->GetRenderObject()->baseTex = metalTex;
	object->GetRenderObject()->bumpTex = testBumpTex;
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
	
	srand(time(0));
	int test;

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

	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);
	worldFloor = floor;
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
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, bool render, float inverseMass) {
	RayMarchSphere* sphere = new RayMarchSphere();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	if(render)sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	sphere->color = { 1,0,0 };
	sphere->radius = radius;
	sphere->center = position;
	rayMarchSpheres.push_back(sphere);
	spheres.push_back(sphere);
	/*int offset = (rayMarchSpheres.size() - 1) * sizeof(RayMarchSphere);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rayMarchSphereSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(float), &(position.x));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + sizeof(float), sizeof(float), &(position.y));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 2*sizeof(float), sizeof(float), &(position.z));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + 3*sizeof(float), sizeof(float), &(radius));
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/
	return sphere;
}





GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetPosition(position).SetScale(dimensions * 3.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, nullptr, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	InitPaintableTextureOnObject(cube);

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform().SetScale(Vector3(radius, halfHeight, radius)).SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* TutorialGame::AddMonkeyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* monkey = new GameObject();
	

	AABBVolume* volume = new AABBVolume(dimensions);
	monkey->SetBoundingVolume((CollisionVolume*)volume);

	monkey->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	monkey->SetRenderObject(new RenderObject(&monkey->GetTransform(), monkeyMesh, nullptr, basicShader));
	monkey->SetPhysicsObject(new PhysicsObject(&monkey->GetTransform(), monkey->GetBoundingVolume()));

	monkey->GetPhysicsObject()->SetInverseMass(inverseMass);
	monkey->GetPhysicsObject()->InitCubeInertia();

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

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), basicWallMesh, nullptr, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(inverseMass);
	wall->GetPhysicsObject()->InitCubeInertia();

	InitPaintableTextureOnObject(wall);

	world->AddGameObject(wall);

	return wall;
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

//GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) { //Origonal
//	float meshSize		= 1.0f;
//	float inverseMass	= 0.5f;
//
//	GameObject* character = new GameObject();
//	SphereVolume* volume  = new SphereVolume(1.0f);
//
//	character->SetBoundingVolume((CollisionVolume*)volume);
//
//	character->GetTransform()
//		.SetScale(Vector3(meshSize, meshSize, meshSize))
//		.SetPosition(position);
//
//	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
//	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
//
//	character->GetPhysicsObject()->SetInverseMass(inverseMass);
//	character->GetPhysicsObject()->InitSphereInertia();
//
//	world->AddGameObject(character);
//
//	return character;
//}
GameObject* TutorialGame::AddDebugTriangleToWorld(const Vector3& position) {
	GameObject* triangle = new GameObject();

	triangle->GetTransform().SetScale(Vector3(100, 100, 1));
	triangle->GetTransform().SetPosition(position);

	triangle->SetRenderObject(new RenderObject(&triangle->GetTransform(), triangleMesh, testCollisionTex, basicShader));
	world->AddGameObject(triangle);
	return triangle;
}

playerTracking* TutorialGame::AddPlayerToWorld(const Vector3& position, Quaternion & orientation) {
	float meshSize = 2.0f;
	float inverseMass = 0.8f;

	playerTracking* character = new playerTracking();
	AABBVolume* volume = new AABBVolume(Vector3{ 2,2,2 });

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position).SetOrientation(orientation);
	//character->GetTransform().setGoatID(7);
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

}


Projectile* TutorialGame::useNewBullet(playerTracking* passedPlayableCharacter) {
	gun wepType = passedPlayableCharacter->getWeponType();

	Projectile* sphere = objectpool->GetObjectW();
	float bulletsInverseMass = sphere->getWeight();
	float radius = sphere->getProjectileRadius();
	//Vector3 playerDirectionVector = (Vector3::Cross((passedPlayableCharacter->GetTransform().GetOrientation().ToEuler()) , Vector3 {1,0,0})).Normalised();
	Vector3 playerDirectionVector = (passedPlayableCharacter->GetTransform().GetOrientation() * Vector3 { 0, 0, -1 });
	Vector3 sphereSize = { radius,radius,radius };
	Vector3 position = passedPlayableCharacter->GetTransform().GetPosition();
	SphereVolume* volume = new SphereVolume(radius);
	sphere->setBulletDirectionVector(playerDirectionVector);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize);
	sphere->GetTransform().SetPosition(position - Vector3{ 0,0,10 });
	sphere->GetTransform().SetPosition((position)-(Vector3{ 0,0,10 }));

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
	AddCubeToWorld(Vector3(0.0f, 15.0f, 0.0f), Vector3(2.5f, 2.5f, 2.5f), 0.5f);
	AddCapsuleToWorld(Vector3(0.0f, 15.0f, 5.0f), 2.5, 2.5);
	//auto q = Quaternion();
	////TODO
	//selectionObject = AddPlayerToWorld(Vector3(0, 5, 0), q);
	//AddEnemyToWorld(Vector3(5, 5, 0));
	//AddBonusToWorld(Vector3(10, 5, 0));
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


void TutorialGame::DispatchComputeShaderForEachTriangle(GameObject* object, Vector3 spherePosition, float sphereRadius) {

	Matrix4 modelMatrix = object->GetTransform().GetMatrix();
	MESH_TRIANGLES_AND_UVS tris = object->GetRenderObject()->GetMesh()->GetAllTrianglesAndUVs();//TODO use vao instead
	triComputeShader->Bind();

	Vector2 maskDims = object->GetRenderObject()->maskDimensions;
	
	/*glBindTexture(GL_TEXTURE_2D, (((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID()));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, maskDims.x, maskDims.y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, zeros.data());
	glBindTexture(GL_TEXTURE_2D, 0);*/
	
#ifdef TRI_DEBUG
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, ((OGLTexture*)(object->GetRenderObject()->triDataTex))->GetObjectID(), 0, GL_FALSE, NULL, GL_READ_WRITE, GL_RGBA16F);
#endif

	glActiveTexture(GL_TEXTURE1);
	glBindImageTexture(1, ((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID(), 0, GL_FALSE, NULL, GL_READ_WRITE, GL_R8UI);

	int radiusLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "sphereRadius");
	int centerLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "sphereCenter");
	int textureWidthLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "textureWidth");
	int textureHeightLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "textureHeight");
	int isComplexLocation = glGetUniformLocation(triComputeShader->GetProgramID(), "isComplex");
	glUniform1f(radiusLocation, sphereRadius);
	glUniform3fv(centerLocation,1, spherePosition.array);
	glUniform1i(textureWidthLocation, object->GetRenderObject()->maskDimensions.x);
	glUniform1i(textureHeightLocation, object->GetRenderObject()->maskDimensions.y);
	glUniform1i(isComplexLocation, object->GetRenderObject()->isComplex);
	
	//TODO change all of this to use vao
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
	std::array<float, MAX_TRIS * 15> emptyArray{};
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * MAX_TRIS * 15, emptyArray.data());

	for (int i = 0; i < tris.size(); i++)
	{
		int offset = i * sizeof(float) * 3 * 5;
		int numFloatsSent = 0;
		std::tuple<std::array<Vector3, 3>, std::array<Vector2, 3>> tri = tris[i];
		Vector3 vertA = modelMatrix * std::get<0>(tri)[0];
		Vector3 vertB = modelMatrix * std::get<0>(tri)[1];
		Vector3 vertC = modelMatrix * std::get<0>(tri)[2];

		Vector2 uvA = std::get<1>(tri)[0];
		Vector2 uvB = std::get<1>(tri)[1];
		Vector2 uvC = std::get<1>(tri)[2];
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float),sizeof(float),&(vertA.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float),sizeof(float),&(vertA.y));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float),sizeof(float),&(vertA.z));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float),sizeof(float),&(uvA.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float),sizeof(float),&(uvA.y));

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(vertB.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(vertB.y));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(vertB.z));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(uvB.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(uvB.y));

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(vertC.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(vertC.y));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(vertC.z));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(uvC.x));
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset + numFloatsSent++ * sizeof(float), sizeof(float), &(uvC.y));
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	triComputeShader->Execute(object->GetRenderObject()->GetMesh()->GetIndexCount()/64+1, 1, 1);//todo change number of thread groups
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	triComputeShader->Unbind();
}

void NCL::CSC8503::TutorialGame::SetUpTriangleSSBOAndDataTexture()
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
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16F, MAX_TRIS, 0, GL_RGBA, GL_FLOAT, nullptr);
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