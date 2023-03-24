#pragma once

#include "GameManager.h"

namespace NCL::CSC8503 {

	struct PBRTextures;
	
	struct TextureThing;

	class PaintSphere;
	class PhysicsSystem;
	class playerTracking;
	class RespawnPoint;
	class PropSystem;

	enum TeamID {
		TEAM_DEFAULT,
		TEAM_RED,
		TEAM_BLUE,
		TEAM_GREEN,
		TEAM_YELLOW,
	};

	class SinglePlayerGame : public GameBase
	{
	public:
		SinglePlayerGame(GameManager* manager, GameWorld* world, GameTechRenderer* renderer);
		~SinglePlayerGame();
		void Update(float dt) override;
	private:
		PhysicsSystem* physics = nullptr;
		PropSystem* propSystem = nullptr;

		bool controllingCamera = false;

		std::vector<PaintSphere*> rayMarchSpheres;

		Vector3 testSphereCenter;
		float testSphereRadius;

		unsigned int highestTriCount = 0;
		MeshGeometry* sphereMesh = nullptr;
		MeshGeometry* floorMesh = nullptr;
		MeshGeometry* playerMesh = nullptr;
		MeshGeometry* cubeMesh = nullptr;

		
		static void LoadTextureThread(const std::string& name, TextureBase** ptr);
		TextureBase* basicTex = nullptr;

		

		PBRTextures* crystalPBR;
		PBRTextures* spaceShipPBR;
		PBRTextures* rockPBR;
		PBRTextures* grassWithWaterPBR;
		PBRTextures* fencePBR;

		ShaderBase* basicShader = nullptr;

		GameObject* lockedObject = nullptr;

		void DrawImGuiSettings();

		void LoadPBRTextures();

		void InitPaintableTextureOnObject(GameObject* object);

		void InitCamera();

		void InitGameWorld();
		playerTracking* playerObject = nullptr;
		playerTracking* AddPlayerToWorld(const Vector3& position, Quaternion& orientation, int team = 0, RespawnPoint* rp = nullptr);
		void AddMapToWorld();
		GameObject* AddFloorToWorld(const Vector3& position, const Vector3& scale);

		std::vector<GameObject*> walls;
		GameObject* AddWallToWorld2(const Vector3& position, Vector3 dimensions);

		ShaderBase* characterShader;
		MeshMaterial* playerMaterial;
		std::vector<RenderObject*> animatedObjects;
	};
	

}