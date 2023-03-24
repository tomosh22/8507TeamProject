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
		void Render() override;
	private:

		bool controllingCamera = false;

		std::vector<PaintSphere*> rayMarchSpheres;

		Vector3 testSphereCenter;
		float testSphereRadius;

		MeshGeometry* sphereMesh = nullptr;
		MeshGeometry* floorMesh = nullptr;
		MeshGeometry* playerMesh = nullptr;
		MeshGeometry* cubeMesh = nullptr;
		MeshGeometry* tyresMesh = nullptr;

		TextureBase* basicTex = nullptr;
		PBRTextures* crystalPBR;
		PBRTextures* spaceShipPBR;
		PBRTextures* rockPBR;
		PBRTextures* grassWithWaterPBR;
		PBRTextures* fencePBR;
		PBRTextures* tyresPBR;

		ShaderBase* basicShader = nullptr;

		GameObject* lockedObject = nullptr;
		GameObject* theFloor = nullptr;

		void DrawImGuiSettings();

		void LoadPBRTextures();

		void InitPaintableTextureOnObject(GameObject* object);

		void InitCamera();

		void InitGameWorld();
		playerTracking* playerObject = nullptr;
		playerTracking* AddPlayerToWorld(const Vector3& position, Quaternion& orientation, int team = 0, RespawnPoint* rp = nullptr);
		void AddStructureToWorld();
		GameObject* AddLadderToWorld(const Vector3& position, float height, bool rotated);
		GameObject* AddTyresToWorld(const Vector3& position, Vector3 dimensions, float inverseMass);
		void AddMapToWorld();
		void AddTowersToWorld();
		void AddPowerUps();
		void AddRespawnPoints();
		GameObject* AddFloorToWorld(const Vector3& position, const Vector3& scale);
		GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass);


		std::vector<GameObject*> walls;
		GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions);

		ShaderBase* characterShader;
		MeshMaterial* playerMaterial;
		std::vector<RenderObject*> animatedObjects;
	};
	

}