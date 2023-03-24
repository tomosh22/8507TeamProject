#include "SinglePlayerGame.h"

#include "RenderObject.h"
#include "PhysicsObject.h"
#include "TextureLoader.h"
#include "PropSystem.h"

#include <unordered_map>
#include <thread>
#include <mutex>

#include "playerTracking.h"

using namespace NCL;
using namespace CSC8503;



std::vector<TextureThing> singlePlayerThings;
std::mutex singlePlayerTexturesMutex;


void SinglePlayerLoadTextureThread(const std::string& name, TextureBase** ptr) {
	char* texData = nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
	int flags = 0;
	TextureLoader::LoadTexture(name, texData, width, height, channels, flags);

	TextureThing thing(texData, width, height, channels, flags, ptr);
	singlePlayerTexturesMutex.lock();
	singlePlayerThings.push_back(thing);
	singlePlayerTexturesMutex.unlock();
}


namespace NCL::CSC8503 {

	SinglePlayerGame::SinglePlayerGame(GameManager* manager, GameWorld* world, GameTechRenderer* renderer) :
		GameBase(manager, world, renderer)
	{
		testSphereCenter = Vector3(0, 0, 0);
		testSphereRadius = 10;

		

		//Start the game with the camera enabled
		Window::GetWindow()->SeizeMouse(true);
		controllingCamera = true;

		//Basic reosurces
		sphereMesh = renderer->LoadMesh("sphere.msh");
		floorMesh = renderer->LoadMesh("Corridor_Floor_Basic.msh");
		playerMesh = renderer->LoadMesh("Character/Character.msh");
		playerMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
		cubeMesh = renderer->LoadMesh("cube.msh");
		tyresMesh = renderer->LoadMesh("tyres.msh");

		basicTex = renderer->LoadTexture("checkerboard.png");
		basicShader = renderer->LoadShader("scene.vert", "scene.frag", "scene.tesc", "scene.tese");

		playerMaterial = new MeshMaterial("Character/Character.mat");

		LoadPBRTextures();
		
		world->ClearAndErase();
		

		InitCamera();

		InitGameWorld();
	}

	void SinglePlayerGame::Update(float dt)
	{
		//Update inputs
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q))
		{
			controllingCamera = !controllingCamera;
			if (!controllingCamera)
			{
				Window::GetWindow()->SeizeMouse(false);
			}
			else
			{
				Window::GetWindow()->SeizeMouse(true);
			}
		}
		else if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
		{
			gameManager->CloseGame();
		}

		if (controllingCamera)
		{
			world->GetMainCamera()->UpdateCamera(dt);
		}

		DrawImGuiSettings();

		//Update shperes
		std::function<Vector3(float)> sinFunctions[] = {
			[](float timePassed) {return Vector3(std::sin(timePassed) * 20,0,0); },
			[](float timePassed) {return Vector3(std::sin(timePassed) * -20,0,0); },
			[](float timePassed) {return Vector3(0, std::sin(timePassed) * 20, 0); },
			[](float timePassed) {return Vector3(0, std::sin(timePassed) * -20, 0); },
			[](float timePassed) {return Vector3(0, 0, std::sin(timePassed) * 20); },
			[](float timePassed) {return Vector3(0, 0, std::sin(timePassed) * -20); }
		};

		Vector3 colours[] = {
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 },

			{ 0, 1, 1 },
			{ 1, 0, 1 },
			{ 1, 1, 0 },
		};

		for (size_t i = 0; i < rayMarchSpheres.size(); i++)
		{
			PaintSphere* sphere = rayMarchSpheres[i];
			sphere->GetTransform().SetPosition(sinFunctions[i % 6](renderer->timePassed) + Vector3(20, 20, 20));
			sphere->center = sphere->GetTransform().GetPosition();
			sphere->radius = sphere->GetTransform().GetScale().x;
			sphere->color = colours[i % 6];
		}

	}

	void SinglePlayerGame::DrawImGuiSettings()
	{
		Vector3 camPos = world->GetMainCamera()->GetPosition();
		std::string camPosStr = std::to_string(camPos.x) + " "
			+ std::to_string(camPos.y) + " " + std::to_string(camPos.z);

		ImGui::Begin("Single Player Game");

		ImGui::Text(camPosStr.c_str());

		if (ImGui::TreeNode("Ray Marching")) {
			auto& settings = renderer->rayMarchingSettings;
			ImGui::Checkbox("Raymarch", &settings.enabled);
			ImGui::SliderInt("Max Steps", &settings.maxSteps, 1, 1000);
			ImGui::SliderFloat("Hit Distance", &settings.hitDistance, 0, 1);
			ImGui::SliderFloat("No Hit Distance", &settings.noHitDistance, 0, 1000);
			ImGui::SliderFloat("Debug Value", &settings.debugValue, -1, 10);
			ImGui::Checkbox("Depth Test", &settings.rayMarchDepthTest);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Paint Testing")) {
			ImGui::SliderFloat3("Position", testSphereCenter.array, -200, 500);
			ImGui::SliderFloat("Sphere Radius", &testSphereRadius, 0, 2000);
			ImGui::SliderFloat("Noise Scale", &renderer->noiseScale, 0, 10);
			ImGui::SliderFloat("Noise Offset Size", &renderer->noiseOffsetSize, 0, 0.1f);
			ImGui::SliderFloat("Noise Normal Strength", &renderer->noiseNormalStrength, 0, 10);
			ImGui::SliderFloat("Noise Normal Multiplier", &renderer->noiseNormalNoiseMult, 0, 10);
			ImGui::SliderFloat("Time Scale", &renderer->noiseTimeScale, 0, 1);

			if (ImGui::Button("Move to Center")) testSphereCenter = Vector3(0, 0, 0);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("PBR")) {
			auto& settings = renderer->pbrSettings;
			ImGui::Checkbox("Bump Map", &settings.useBumpMap);
			ImGui::Checkbox("Metallic Map", &settings.useMetallicMap);
			ImGui::Checkbox("Roughness Map", &settings.useRoughnessMap);
			ImGui::Checkbox("Height Map", &settings.useHeightMap);
			ImGui::Checkbox("Emission Map", &settings.useEmissionMap);
			ImGui::Checkbox("AO Map", &settings.useAOMap);
			ImGui::Checkbox("Opacity Map", &settings.useOpacityMap);
			ImGui::Checkbox("Gloss Map", &settings.useGlossMap);
			ImGui::SliderFloat("Heightmap Strength", &settings.heightMapStrength, 0, 10);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("FXAA")) {
			ImGui::Checkbox("Use FXAA", &renderer->fxaaEnabled);
			ImGui::Checkbox("Edge Detection", &renderer->fxaaEdgeDetection);
			ImGui::SliderFloat("Edge Threshold", &renderer->fxaaEdgeThreshold, 0, 0.5);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("HDR")) {
			ImGui::Checkbox("Tone map", &renderer->toneMap);
			ImGui::SliderFloat("Edge Threshold", &renderer->exposure, -10, 10);
			ImGui::TreePop();
		}

		ImGui::SliderFloat3("Light Position", renderer->lightPosition.array, -200, 200);

		ImGui::End();
	}

	void SinglePlayerGame::InitCamera() {
		world->GetMainCamera()->SetNearPlane(0.1f);
		world->GetMainCamera()->SetFarPlane(500.0f);
		world->GetMainCamera()->SetPitch(-15.0f);
		world->GetMainCamera()->SetYaw(315.0f);
		world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
		lockedObject = nullptr;
	}

	void SinglePlayerGame::Render()
	{

		for (size_t i = 0; i < rayMarchSpheres.size(); i++)
		{
			Vector3 position = rayMarchSpheres[i]->GetTransform().GetPosition();
			Vector3 color = rayMarchSpheres[i]->color;
			float radius = rayMarchSpheres[i]->radius;

			renderer->SubmitRayMarchedSphere(position, color, radius);
		}

		//renderer->ApplyPaintTo(wall, testSphereCenter, testSphereRadius, 7);
		world->OperateOnContents([&](GameObject* object)
			{
				if (!object->GetRenderObject() || !object->GetRenderObject()->isPaintable) return;
				renderer->ApplyPaintTo(object, testSphereCenter, testSphereRadius, 7);
			});
	}

	void SinglePlayerGame::LoadPBRTextures() {
		std::vector<std::thread> threads;
		crystalPBR = new PBRTextures();
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_diffuse.jpg", &(crystalPBR->base)));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_normal.jpg", &crystalPBR->bump));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_metallic.jpg", &crystalPBR->metallic));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_roughness.jpg", &crystalPBR->roughness));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_height.jpg", &crystalPBR->heightMap));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_emissive.jpg", &crystalPBR->emission));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_ao.jpg", &crystalPBR->ao));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_opacity.jpg", &crystalPBR->opacity));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_glossiness.jpg", &crystalPBR->gloss));

		spaceShipPBR = new PBRTextures();
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_diffuse.jpg", &spaceShipPBR->base));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_normal.jpg", &spaceShipPBR->bump));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_metalness.jpg", &spaceShipPBR->metallic));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_roughness.jpg", &spaceShipPBR->roughness));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_height.jpg", &spaceShipPBR->heightMap));
		spaceShipPBR->emission = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_ao.jpg", &spaceShipPBR->ao));
		spaceShipPBR->opacity = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_glossiness.jpg", &spaceShipPBR->gloss));

		rockPBR = new PBRTextures();
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_diffuse.jpg", &rockPBR->base));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_normal.jpg", &rockPBR->bump));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_metallic.jpg", &rockPBR->metallic));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_roughness.jpg", &rockPBR->roughness));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_height.jpg", &rockPBR->heightMap));
		rockPBR->emission = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_ao.jpg", &rockPBR->ao));
		rockPBR->opacity = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_glossiness.jpg", &rockPBR->gloss));

		grassWithWaterPBR = new PBRTextures();
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_diffuse.jpg", &grassWithWaterPBR->base));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_normal.jpg", &grassWithWaterPBR->bump));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_metallic.jpg", &grassWithWaterPBR->metallic));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_roughness.jpg", &grassWithWaterPBR->roughness));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_height.jpg", &grassWithWaterPBR->heightMap));
		grassWithWaterPBR->emission = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_ao.jpg", &grassWithWaterPBR->ao));
		grassWithWaterPBR->opacity = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_glossiness.jpg", &grassWithWaterPBR->gloss));

		fencePBR = new PBRTextures();
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_diffuse.jpg", &fencePBR->base));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_normal.jpg", &fencePBR->bump));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_metallic.jpg", &fencePBR->metallic));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_roughness.jpg", &fencePBR->roughness));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_height.jpg", &fencePBR->heightMap));
		fencePBR->emission = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_ao.jpg", &fencePBR->ao));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_opacity.jpg", &fencePBR->opacity));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_glossiness.jpg", &fencePBR->gloss));

		tyresPBR = new PBRTextures();
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/tyres/diffuse 2k.jpeg", &tyresPBR->base));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/tyres/normal 2k.jpeg", &tyresPBR->bump));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/tyres/metalic 2k.jpeg", &tyresPBR->metallic));
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/tyres/roughness 2k.jpeg", &tyresPBR->roughness));
		tyresPBR->heightMap = nullptr;
		tyresPBR->emission = nullptr;
		threads.push_back(std::thread(SinglePlayerLoadTextureThread, "PBR/tyres/AO 2k.jpeg", &tyresPBR->ao));
		tyresPBR->opacity = nullptr;
		tyresPBR->gloss = nullptr;

		for (std::thread& thread : threads) {
			thread.join();
		}

		for (TextureThing& thing : singlePlayerThings)
		{
			TextureBase* tex = OGLTexture::RGBATextureFromData(thing.texData, thing.width, thing.height, thing.channels);
			*(thing.myPointer) = (tex);
			free(thing.texData);
		}
		threads.clear();
	}

	void SinglePlayerGame::InitGameWorld() {
		Quaternion q;
		//playerObject = AddPlayerToWorld(Vector3(0, 5.0f, 10.0f), q);
		//playerObject->SetTeamId(TEAM_RED);

		AddMapToWorld();
		AddStructureToWorld();
		AddTowersToWorld();

		//AddPowerUps();
		//AddRespawnPoints();


	}

	void SinglePlayerGame::InitPaintableTextureOnObject(GameObject* object) {
		int w, h;
		
		w = (int)(object->GetTransform().GetScale().x * TEXTURE_DENSITY);
		h = (int)(object->GetTransform().GetScale().z * TEXTURE_DENSITY);
		
		GameTechRenderer::AttachPaintMask(object, w, h);
	}

	GameObject* SinglePlayerGame::AddTyresToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
		GameObject* tyres = new GameObject();

		AABBVolume* volume = new AABBVolume(dimensions);
		tyres->SetBoundingVolume((CollisionVolume*)volume);

		tyres->GetTransform()
			.SetPosition(position)
			.SetScale(dimensions * 2)
			.SetOrientation(Quaternion::AxisAngleToQuaterion({ 1,0,0 }, 270));

		tyres->SetRenderObject(new RenderObject(&tyres->GetTransform(), tyresMesh, nullptr, basicShader));
		tyres->SetPhysicsObject(new PhysicsObject(&tyres->GetTransform(), tyres->GetBoundingVolume()));

		tyres->GetPhysicsObject()->SetInverseMass(inverseMass);
		tyres->GetPhysicsObject()->InitCubeInertia();

		InitPaintableTextureOnObject(tyres);

		tyres->GetRenderObject()->isPaintable = true;
		tyres->GetRenderObject()->pbrTextures = tyresPBR;
		tyres->GetRenderObject()->useTriplanarMapping = false;

		world->AddGameObject(tyres);
		tyres->SetName("tyres");

		return tyres;
	}

	GameObject* SinglePlayerGame::AddFloorToWorld(const Vector3& position, const Vector3& scale) {
		GameObject* floor = new GameObject();

		AABBVolume* volume = new AABBVolume(scale);
		floor->SetBoundingVolume((CollisionVolume*)volume);
		floor->GetTransform()
			.SetScale(scale * 2)
			.SetPosition(position);

		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), floorMesh, nullptr, basicShader));

		InitPaintableTextureOnObject(floor);
		glObjectLabel(GL_TEXTURE, static_cast<OGLTexture*>(floor->GetRenderObject()->maskTex)->GetObjectID(), 10, "Floor Mask");

		floor->GetRenderObject()->useHeightMap = true;
		floor->GetRenderObject()->isPaintable = true;
		floor->GetRenderObject()->pbrTextures = crystalPBR;

		floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

		floor->GetPhysicsObject()->SetInverseMass(0);
		floor->GetPhysicsObject()->InitCubeInertia();

		world->AddGameObject(floor);

		floor->SetName("floor");
		return floor;
	}

	GameObject* SinglePlayerGame::AddWallToWorld(const Vector3& position, Vector3 dimensions)
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
		myWall->GetRenderObject()->pbrTextures = spaceShipPBR;

		world->AddGameObject(myWall);

		return myWall;
	}

	GameObject* SinglePlayerGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
		GameObject* cube = new GameObject();

		OBBVolume* volume = new OBBVolume(dimensions);
		cube->SetBoundingVolume((CollisionVolume*)volume);

		cube->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);

		cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, nullptr, basicShader));
		cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

		cube->GetPhysicsObject()->SetInverseMass(inverseMass);
		cube->GetPhysicsObject()->InitCubeInertia();

		InitPaintableTextureOnObject(cube);
		cube->GetRenderObject()->pbrTextures = rockPBR;

		world->AddGameObject(cube);
		cube->SetName("cube");
		return cube;
	}

	GameObject* SinglePlayerGame::AddLadderToWorld(const Vector3& position, float height, bool rotated)
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

	void SinglePlayerGame::AddStructureToWorld()
	{
		//middle structure
		//pillars
		AddCubeToWorld({ 12, 7, 8 }, { 1, 7, 1 }, 0.0f);
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
		AddLadderToWorld({ 88.5, 7, 0 }, 7.0f, false);

		//pillars
		AddCubeToWorld({ -87, 7, 8 }, { 1, 7, 1 }, 0.0f);
		AddCubeToWorld({ -63, 7, 8 }, { 1, 7, 1 }, 0.0f);
		AddCubeToWorld({ -87, 7, -8 }, { 1, 7, 1 }, 0.0f);
		AddCubeToWorld({ -63, 7, -8 }, { 1, 7, 1 }, 0.0f);

		//platform 
		AddCubeToWorld({ -75, 13, 0 }, { 13, 1, 9 }, 0.0f);
		AddLadderToWorld({ -88.5, 7, 0 }, 7.0f, false);

		//paths betweeen platforms
		AddCubeToWorld({ 38, 13, 0 }, { 25, 1, 6 }, 0.0f);
		AddCubeToWorld({ -38, 13, 0 }, { 25, 1, 6 }, 0.0f);

		//walls on platforms
		AddCubeToWorld({ 0, 15.5, 8 }, { 13, 1.5, 1 }, 0.0f);
		AddCubeToWorld({ 0, 15.5, -8 }, { 13, 1.5, 1 }, 0.0f);

		AddCubeToWorld({ -75, 15.5, 8 }, { 13, 1.5, 1 }, 0.0f);
		AddCubeToWorld({ -75, 15.5, -8 }, { 13, 1.5, 1 }, 0.0f);

		AddCubeToWorld({ 75, 15.5, 8 }, { 13, 1.5, 1 }, 0.0f);
		AddCubeToWorld({ 75, 15.5, -8 }, { 13, 1.5, 1 }, 0.0f);
	}

	void SinglePlayerGame::AddTowersToWorld()
	{
		AddCubeToWorld({ 70, 12.5, 150 }, { 2.5 , 12.5, 2.5 }, 0.0f)->GetRenderObject()->pbrTextures = rockPBR;
		AddLadderToWorld({ 70, 12.5, 153 }, 12.5f, true)->GetRenderObject()->pbrTextures = rockPBR;

		AddCubeToWorld({ -70, 12.5, -150 }, { 2.5 , 12.5, 2.5 }, 0.0f)->GetRenderObject()->pbrTextures = rockPBR;
		AddLadderToWorld({ -70, 12.5, -153 }, 12.5f, true)->GetRenderObject()->pbrTextures = rockPBR;
	}

	void SinglePlayerGame::AddMapToWorld() {
		//floor and enclosing walls 
		theFloor = AddFloorToWorld({ 0, 0, 0 }, { 100, 1, 200 });

		//visible walls
		walls.push_back(AddWallToWorld({ 100, 5, 0 }, { 1, 5, 200 }));
		walls.push_back(AddWallToWorld({ -100, 5, 0 }, { 1, 5, 200 }));

		walls.push_back(AddWallToWorld({ 0, 5, 200 }, { 100, 5, 1 }));
		walls.push_back(AddWallToWorld({ 0, 5, -200 }, { 100, 5, 1 }));

		//invisible walls
		AddWallToWorld({ 100, 5, 0 }, { 1, 50, 200 })->SetRenderObject(nullptr);
		AddWallToWorld({ -100, 5, 0 }, { 1, 50, 200 })->SetRenderObject(nullptr);

		AddWallToWorld({ 0, 5, 200 }, { 100, 50, 1 })->SetRenderObject(nullptr);
		AddWallToWorld({ 0, 5, -200 }, { 100, 50, 1 })->SetRenderObject(nullptr);

		//dividing walls
		/*walls.push_back(AddWallToWorld2({ -50, 7, 0 }, { 1, 7, 100 }));
		walls.push_back(AddWallToWorld2({ 50, 7, 0 }, { 1, 7, 100 }));*/

		walls.push_back(AddWallToWorld({ -50, 7, 50 }, { 1, 7, 40 }));
		walls.push_back(AddWallToWorld({ -50, 7, -50 }, { 1, 7, 40 }));

		walls.push_back(AddWallToWorld({ 50, 7, 50 }, { 1, 7, 40 }));
		walls.push_back(AddWallToWorld({ 50, 7, -50 }, { 1, 7, 40 }));

		//back cover walls
		walls.push_back(AddWallToWorld({ 0, 5, 125 }, { 25, 5, 1 }));
		walls.push_back(AddWallToWorld({ 0, 5, -125 }, { 25, 5, 1 }));

		//middle low cover walls
		walls.push_back(AddWallToWorld({ 30, 2.5, 75 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ -30, 2.5, 75 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ 0, 2.5, 75 }, { 10, 2.5, 1 }));

		walls.push_back(AddWallToWorld({ -15, 2.5, 35 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ 15, 2.5, 35 }, { 10, 2.5, 1 }));

		walls.push_back(AddWallToWorld({ 30, 2.5, -75 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ -30, 2.5, -75 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ 0, 2.5, -75 }, { 10, 2.5, 1 }));

		walls.push_back(AddWallToWorld({ -15, 2.5, -35 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ 15, 2.5, -35 }, { 10, 2.5, 1 }));

		//side low cover walls 
		walls.push_back(AddWallToWorld({ 75, 2.5, 50 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ -75, 2.5, 50 }, { 10, 2.5, 1 }));

		walls.push_back(AddWallToWorld({ 75, 2.5, -50 }, { 10, 2.5, 1 }));
		walls.push_back(AddWallToWorld({ -75, 2.5, -50 }, { 10, 2.5, 1 }));

		AddTyresToWorld({ 40,1.5,50 }, { 2,2,2 }, 0);
		AddTyresToWorld({ -40,1.5,50 }, { 2,2,2 }, 0);
		AddTyresToWorld({ 40,1.5,-50 }, { 2,2,2 }, 0);
		AddTyresToWorld({ -40,1.5,-50 }, { 2,2,2 }, 0);

		AddTyresToWorld({ 10,1.5,-15 }, { 2,2,2 }, 0);
		AddTyresToWorld({ -10,1.5,-15 }, { 2,2,2 }, 0);
		AddTyresToWorld({ 10,1.5,15 }, { 2,2,2 }, 0);
		AddTyresToWorld({ -10,1.5,15 }, { 2,2,2 }, 0);

		//AddPowerUps();
	}

	playerTracking* SinglePlayerGame::AddPlayerToWorld(const Vector3& position, Quaternion& orientation, int team, RespawnPoint* rp) {
		float meshSize = 2.0f;
		float inverseMass = 0.3f;

		playerTracking* character = new playerTracking(world);
		AABBVolume* volume = new AABBVolume(Vector3{ 2,2,2 });

		character->SetBoundingVolume((CollisionVolume*)volume);

		character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position).SetOrientation(orientation);
		//character->GetTransform().setGoatID(7);
		character->setImpactAbsorbtionAmount(0.9f);

		character->SetRenderObject(new RenderObject(&character->GetTransform(), playerMesh, nullptr, characterShader));
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
		//InitPaintableTextureOnObject(character); //todo do we need this???

		world->AddGameObject(character);
		character->SetName("character");
		character->SetTeamId(1);
		return character;
	}

	SinglePlayerGame::~SinglePlayerGame()
	{

	}

}