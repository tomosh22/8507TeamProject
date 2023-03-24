#include "GraphicsGame.h"

#include "RenderObject.h"
#include "PhysicsObject.h"
#include "TextureLoader.h"

#include <unordered_map>
#include <thread>
#include <mutex>

using namespace NCL;
using namespace CSC8503;

struct TextureThing {
	char* texData = nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
	int flags = 0;

	TextureBase** myPointer;

	TextureThing(char* texData, int width, int height, int channels, int flags, TextureBase** myPointer) : texData(texData), width(width), height(height), channels(channels), flags(flags), myPointer(myPointer) {}
};

std::vector<TextureThing> things;
std::mutex texturesMutex;


void LoadTextureThread(const std::string& name, TextureBase** ptr) {
	char* texData = nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
	int flags = 0;
	TextureLoader::LoadTexture(name, texData, width, height, channels, flags);

	TextureThing thing(texData, width, height, channels, flags, ptr);
	texturesMutex.lock();
	things.push_back(thing);
	texturesMutex.unlock();
}

namespace NCL::CSC8503 {

	GraphicsGame::GraphicsGame(GameManager* manager, GameWorld* world, GameTechRenderer* renderer) :
		GameBase(manager, world, renderer)
	{
		testSphereCenter = Vector3(0, 0, 0);
		testSphereRadius = 10;

		//Start the game with the camera enabled
		controllingCamera = true;
		Window::GetWindow()->SeizeMouse(controllingCamera);

		//Basic reosurces
		sphereMesh = renderer->LoadMesh("sphere.msh");
		floorMesh = renderer->LoadMesh("Corridor_Floor_Basic.msh");
		basicTex = renderer->LoadTexture("checkerboard.png");
		basicShader = renderer->LoadShader("scene.vert", "scene.frag", "scene.tesc", "scene.tese");

		ironDiffuse = renderer->LoadTexture("PBR/rustediron2_basecolor.png");
		ironBump = renderer->LoadTexture("PBR/rustediron2_normal.png");
		ironMetallic = renderer->LoadTexture("PBR/rustediron2_metallic.png");
		ironRoughness = renderer->LoadTexture("PBR/rustediron2_roughness.png");

		std::vector<std::thread> threads;

		crystalPBR = new PBRTextures();
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_diffuse.jpg", &(crystalPBR->base)));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_normal.jpg", &crystalPBR->bump));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_metallic.jpg", &crystalPBR->metallic));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_roughness.jpg", &crystalPBR->roughness));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_height.jpg", &crystalPBR->heightMap));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_emissive.jpg", &crystalPBR->emission));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_ao.jpg", &crystalPBR->ao));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_opacity.jpg", &crystalPBR->opacity));
		threads.push_back(std::thread(LoadTextureThread, "PBR/crystal2k/violet_crystal_43_04_glossiness.jpg", &crystalPBR->gloss));

		spaceShipPBR = new PBRTextures();
		threads.push_back(std::thread(LoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_diffuse.jpg", &spaceShipPBR->base));
		threads.push_back(std::thread(LoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_normal.jpg", &spaceShipPBR->bump));
		threads.push_back(std::thread(LoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_metalness.jpg", &spaceShipPBR->metallic));
		threads.push_back(std::thread(LoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_roughness.jpg", &spaceShipPBR->roughness));
		threads.push_back(std::thread(LoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_height.jpg", &spaceShipPBR->heightMap));
		spaceShipPBR->emission = nullptr;
		threads.push_back(std::thread(LoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_ao.jpg", &spaceShipPBR->ao));
		spaceShipPBR->opacity = nullptr;
		threads.push_back(std::thread(LoadTextureThread, "PBR/spaceShip1k/white_space_ship_wall_28_66_glossiness.jpg", &spaceShipPBR->gloss));

		rockPBR = new PBRTextures();
		threads.push_back(std::thread(LoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_diffuse.jpg", &rockPBR->base));
		threads.push_back(std::thread(LoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_normal.jpg", &rockPBR->bump));
		threads.push_back(std::thread(LoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_metallic.jpg", &rockPBR->metallic));
		threads.push_back(std::thread(LoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_roughness.jpg", &rockPBR->roughness));
		threads.push_back(std::thread(LoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_height.jpg", &rockPBR->heightMap));
		rockPBR->emission = nullptr;
		threads.push_back(std::thread(LoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_ao.jpg", &rockPBR->ao));
		rockPBR->opacity = nullptr;
		threads.push_back(std::thread(LoadTextureThread, "PBR/rock1k/dirt_with_large_rocks_38_46_glossiness.jpg", &rockPBR->gloss));

		grassWithWaterPBR = new PBRTextures();
		threads.push_back(std::thread(LoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_diffuse.jpg", &grassWithWaterPBR->base));
		threads.push_back(std::thread(LoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_normal.jpg", &grassWithWaterPBR->bump));
		threads.push_back(std::thread(LoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_metallic.jpg", &grassWithWaterPBR->metallic));
		threads.push_back(std::thread(LoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_roughness.jpg", &grassWithWaterPBR->roughness));
		threads.push_back(std::thread(LoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_height.jpg", &grassWithWaterPBR->heightMap));
		grassWithWaterPBR->emission = nullptr;
		threads.push_back(std::thread(LoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_ao.jpg", &grassWithWaterPBR->ao));
		grassWithWaterPBR->opacity = nullptr;
		threads.push_back(std::thread(LoadTextureThread, "PBR/grassWithWater1k/grass_with_water_39_67_glossiness.jpg", &grassWithWaterPBR->gloss));

		fencePBR = new PBRTextures();
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_diffuse.jpg", &fencePBR->base));
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_normal.jpg", &fencePBR->bump));
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_metallic.jpg", &fencePBR->metallic));
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_roughness.jpg", &fencePBR->roughness));
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_height.jpg", &fencePBR->heightMap));
		fencePBR->emission = nullptr;
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_ao.jpg", &fencePBR->ao));
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_opacity.jpg", &fencePBR->opacity));
		threads.push_back(std::thread(LoadTextureThread, "PBR/fence1k/small_old_wooden_fence_47_66_glossiness.jpg", &fencePBR->gloss));

		for (std::thread& thread : threads) {
			thread.join();
		}

		for (TextureThing& thing : things)
		{
			TextureBase* tex = OGLTexture::RGBATextureFromData(thing.texData, thing.width, thing.height, thing.channels);
			*(thing.myPointer) = (tex);
			free(thing.texData);
		}
		threads.clear();

		InitGameWorld();
	}

	GraphicsGame::~GraphicsGame()
	{

	}

	void GraphicsGame::Update(float dt)
	{
		//Update inputs
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q))
		{
			controllingCamera = !controllingCamera;

			Window::GetWindow()->SeizeMouse(controllingCamera);
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

	void GraphicsGame::Render()
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

	void GraphicsGame::DrawImGuiSettings()
	{
		Vector3 camPos = world->GetMainCamera()->GetPosition();
		std::string camPosStr = std::to_string(camPos.x) + " "
			+ std::to_string(camPos.y) + " " + std::to_string(camPos.z);

		ImGui::Begin("NotSplatoon");

		ImGui::Text(camPosStr.c_str());

		if (ImGui::TreeNode("Ray Marching")) {
			auto& settings = renderer->rayMarchingSettings;
			ImGui::Checkbox("Raymarch", &settings.enabled);
			ImGui::SliderInt("Max Steps", &settings.maxSteps, 1, 1000);
			ImGui::SliderFloat("Hit Distance", &settings.hitDistance, 0, 1);
			ImGui::SliderFloat("No Hit Distance", &settings.noHitDistance, 0, 1000);
			ImGui::SliderFloat("Debug Value",&settings.debugValue, -1, 10);
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

	void GraphicsGame::InitGameWorld()
	{
		world->GetMainCamera()->SetNearPlane(0.1f);
		world->GetMainCamera()->SetFarPlane(500.0f);
		world->GetMainCamera()->SetPitch(-15.0f);
		world->GetMainCamera()->SetYaw(315.0f);
		world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));

		for (int i = 0; i < 10; i++)
		{
			AddRayMarchSphereToWorld({ 0,0,0 }, 10);
		}

		GameObject* testSphere0 = AddSphereToWorld({50,50,50}, 10, true, false);
		InitPaintableTextureOnObject(testSphere0);
		testSphere0->GetRenderObject()->pbrTextures = crystalPBR;
		testSphere0->GetRenderObject()->useHeightMap = false;

		GameObject* testSphere1 = AddSphereToWorld({ 50,50,100 }, 10, true, false);
		InitPaintableTextureOnObject(testSphere1);
		testSphere1->GetRenderObject()->pbrTextures = spaceShipPBR;
		testSphere1->GetRenderObject()->useHeightMap = false;

		GameObject* testSphere2 = AddSphereToWorld({ 50,50,150 }, 10, true, false);
		InitPaintableTextureOnObject(testSphere2);
		testSphere2->GetRenderObject()->pbrTextures = rockPBR;
		testSphere2->GetRenderObject()->useHeightMap = false;

		GameObject* testSphere3 = AddSphereToWorld({ 100,50,50 }, 10, true, false);
		InitPaintableTextureOnObject(testSphere3);
		testSphere3->GetRenderObject()->pbrTextures = grassWithWaterPBR;
		testSphere3->GetRenderObject()->useHeightMap = false;

		GameObject* testSphere4 = AddSphereToWorld({ 100,50,100 }, 10, true, false);
		InitPaintableTextureOnObject(testSphere4);
		testSphere4->GetRenderObject()->pbrTextures = fencePBR;
		testSphere4->GetRenderObject()->useHeightMap = false;

		AddFloorToWorld(Vector3(0, -20, 0), { 100,1,100 },false);
		wall = AddFloorToWorld({ 0,25,-50 }, { 100,1,25 }, true);
		wall->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(90, 0, 0));
	}

	void GraphicsGame::InitPaintableTextureOnObject(GameObject* object, bool rotated)
	{
		int w = (int)(object->GetTransform().GetScale().x * TEXTURE_DENSITY);
		int h = (int)(object->GetTransform().GetScale().z * TEXTURE_DENSITY);

		object->GetRenderObject()->isPaintable = true;
		object->GetRenderObject()->maskTex = new OGLTexture();
		object->GetRenderObject()->maskDimensions = { (float)w,(float)h };

		glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	GameObject* GraphicsGame::AddSphereToWorld(const Vector3& position, float radius, bool render, float inverseMass, bool physics) {
		GameObject* sphere = new GameObject();

		Vector3 sphereSize = Vector3(radius, radius, radius);
		SphereVolume* volume = new SphereVolume(radius);
		sphere->SetBoundingVolume((CollisionVolume*)volume);

		sphere->GetTransform()
			.SetScale(sphereSize)
			.SetPosition(position);

		sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
		if (!render) sphere->GetRenderObject()->onlyForShadows = true;

		if (physics)
		{
			sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

			sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
			sphere->GetPhysicsObject()->InitSphereInertia();
		}

		world->AddGameObject(sphere);

		return sphere;
	}

	GameObject* GraphicsGame::AddFloorToWorld(const Vector3& position, const Vector3& scale, bool rotated)
	{
		GameObject* floor = new GameObject();


		AABBVolume* volume = new AABBVolume(scale);
		floor->SetBoundingVolume((CollisionVolume*)volume);
		floor->GetTransform()
			.SetScale(scale * 2)
			.SetPosition(position);

		srand((int)time(0));

		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), floorMesh, nullptr, basicShader));
		floor->GetRenderObject()->pbrTextures = fencePBR;

		InitPaintableTextureOnObject(floor, rotated);
		floor->GetRenderObject()->useHeightMap = true;
		floor->GetRenderObject()->isPaintable = true;

		floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

		floor->GetPhysicsObject()->SetInverseMass(0);
		floor->GetPhysicsObject()->InitCubeInertia();

		world->AddGameObject(floor);

		floor->SetName("floor");
		return floor;
	}

	GameObject* GraphicsGame::AddRayMarchSphereToWorld(const Vector3& position, float radius)
	{
		PaintSphere* sphere = new PaintSphere();

		Vector3 sphereSize = Vector3(radius, radius, radius);
		SphereVolume* volume = new SphereVolume(radius);
		sphere->SetBoundingVolume((CollisionVolume*)volume);

		sphere->GetTransform()
			.SetScale(sphereSize)
			.SetPosition(position);

		//NOTE(Jason): Yes, I know they won't show up in the shadows, but I like it better that way.
		//Feel free to add them back in, if you want.
		//    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
		//    sphere->GetRenderObject()->onlyForShadows = true;

		sphere->color = { 1,0,0 };
		sphere->radius = radius;
		sphere->center = position;
		world->AddGameObject(sphere);
		rayMarchSpheres.push_back(sphere);
		
		return sphere;
	}

}