#pragma once

#include "GameManager.h"

namespace NCL::CSC8503 {

	struct PBRTextures;

	class PaintSphere : public GameObject
	{
	public:
		Vector3 center;
		float radius;
		Vector3 color;
	};

	class GraphicsGame : public GameBase
	{
	private:
		bool controllingCamera = false;

		std::vector<PaintSphere*> rayMarchSpheres;

		Vector3 testSphereCenter;
		float testSphereRadius;

		GameObject* wall;

		MeshGeometry* sphereMesh = nullptr;
		MeshGeometry* floorMesh = nullptr;
		TextureBase* basicTex = nullptr;
		ShaderBase* basicShader = nullptr;

		TextureBase* ironDiffuse = nullptr;
		TextureBase* ironBump = nullptr;
		TextureBase* ironMetallic = nullptr;
		TextureBase* ironRoughness = nullptr;

		PBRTextures* crystalPBR;
		PBRTextures* spaceShipPBR;
		PBRTextures* rockPBR;
		PBRTextures* grassWithWaterPBR;
		PBRTextures* fencePBR;
	private:
		void InitGameWorld();
		void InitPaintableTextureOnObject(GameObject* object, bool rotated = false);

		GameObject* AddRayMarchSphereToWorld(const Vector3& position, float radius);
		GameObject* AddSphereToWorld(const Vector3& position, float radius, bool render, float inverseMass = 10.0f, bool physics = true);
		GameObject* AddFloorToWorld(const Vector3& position, const Vector3& scale, bool rotated);

		void DrawImGuiSettings();
	public:
		GraphicsGame(GameManager* manager, GameWorld* world, GameTechRenderer* renderer);
		~GraphicsGame();

		void Update(float dt) override;
		void Render() override;
	};

}