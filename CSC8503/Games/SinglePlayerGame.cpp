#include "SinglePlayerGame.h"

#include "RenderObject.h"
#include "PhysicsObject.h"
#include "PhysicsSystem.h"
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

		physics = new PhysicsSystem(*world);
		propSystem = new PropSystem(world);

		//Start the game with the camera enabled
		controllingCamera = true;

		//Basic reosurces
		std::vector<MeshGeometry*> meshes;
		sphereMesh = renderer->LoadMesh("sphere.msh", &meshes);
		floorMesh = renderer->LoadMesh("Corridor_Floor_Basic.msh", &meshes);
		playerMesh = renderer->LoadMesh("Character/Character.msh", &meshes);
		playerMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
		cubeMesh = renderer->LoadMesh("cube.msh", &meshes);

		for (MeshGeometry*& mesh : meshes) {
			if (mesh->GetIndexData().size() == 0) std::cout << "mesh doesn't use indices, could be a problem\n";
			if (mesh->GetIndexCount() / 3 > highestTriCount) {
				highestTriCount = mesh->GetIndexCount() / 3;
			}
		}

		basicTex = renderer->LoadTexture("checkerboard.png");
		basicShader = renderer->LoadShader("scene.vert", "scene.frag", "scene.tesc", "scene.tese");

		playerMaterial = new MeshMaterial("Character/Character.mat");

		LoadPBRTextures();
		
		world->ClearAndErase();
		physics->Clear();

		InitCamera();

		InitGameWorld();
	}

	void SinglePlayerGame::Update(float dt)
	{
		//Update inputs
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q))
		{
			controllingCamera = !controllingCamera;
		}
		else if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
		{
			gameManager->CloseGame();
		}

		if (controllingCamera)
		{
			world->GetMainCamera()->UpdateCamera(dt);
		}

		//DrawImGuiSettings();

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

	void SinglePlayerGame::UpdateGame(float dt) {
		
		world->GetMainCamera()->UpdateCamera(dt);
		

		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
	}

	void SinglePlayerGame::InitCamera() {
		world->GetMainCamera()->SetNearPlane(0.1f);
		world->GetMainCamera()->SetFarPlane(500.0f);
		world->GetMainCamera()->SetPitch(-15.0f);
		world->GetMainCamera()->SetYaw(315.0f);
		world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
		lockedObject = nullptr;
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


	}

	void SinglePlayerGame::InitPaintableTextureOnObject(GameObject* object) {
		int w, h;
		
		w = (int)(object->GetTransform().GetScale().x * TEXTURE_DENSITY);
		h = (int)(object->GetTransform().GetScale().z * TEXTURE_DENSITY);
		
		object->GetRenderObject()->isPaintable = true;
		object->GetRenderObject()->maskTex = new OGLTexture();
		glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
		object->GetRenderObject()->maskDimensions = { (float)w,(float)h };
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

		floor->GetRenderObject()->useHeightMap = true;
		floor->GetRenderObject()->isPaintable = true;

		floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

		floor->GetPhysicsObject()->SetInverseMass(0);
		floor->GetPhysicsObject()->InitCubeInertia();

		world->AddGameObject(floor);

		floor->SetName("floor");
		return floor;
	}

	GameObject* SinglePlayerGame::AddWallToWorld2(const Vector3& position, Vector3 dimensions)
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

	void SinglePlayerGame::AddMapToWorld() {
		//floor and enclosing walls 
		AddFloorToWorld({ 0, 0, 0 }, { 100, 1, 200 });

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