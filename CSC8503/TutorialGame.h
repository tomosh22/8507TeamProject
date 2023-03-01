#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"

#include <array>

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& scale, bool rotated = false);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, bool render, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMonkeyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMaxToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			//this was me
			GameObject* AddDebugTriangleToWorld(const Vector3& position);

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;

			//this was me
			MeshGeometry* triangleMesh = nullptr;
			MeshGeometry* monkeyMesh = nullptr;
			MeshGeometry* floorMesh = nullptr;
			MeshGeometry* maxMesh = nullptr;
			MeshGeometry* basicWallMesh = nullptr;

			TextureBase*	basicTex	= nullptr;
			ShaderBase*		basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;

			GameObject* worldFloor = nullptr;
			
			//this was me
			OGLComputeShader* computeShader;
			void RunComputeShader(GameObject* floor,int width, int height, int leftS, int rightS, int topT, int bottomT, int radius,Vector2 center, int teamID);
			OGLShader* quadShader;
			TextureBase* quadTex = nullptr;
			void InitQuadTexture();
			TextureBase* floorTex = nullptr;
			void InitPaintableTextureOnObject(GameObject* object, bool rotated = false);

			void DispatchComputeShaderForEachTriangle(GameObject* object, Vector3 spherePosition, float sphereRadius);
			GLuint triangleSSBO;
			void SetUpTriangleSSBOAndDataTexture();
			OGLComputeShader* triComputeShader;
			//OGLTexture* triDataTex;//1d texture
			//GLuint triangleBoolSSBO;
			bool SphereTriangleIntersection(Vector3 sphereCenter, float sphereRadius, Vector3 v0, Vector3 v1, Vector3 v2, Vector3& intersectionPoint);

			void DispatchComputeShaderForEachPixel();
			OGLComputeShader* rayMarchComputeShader;
			int maxSteps;
			float hitDistance;
			float noHitDistance;
			float debugValue;
			class RayMarchSphere : public GameObject {
			public:
				Vector3 center;
				float radius;
				Vector3 color;
			};
			std::vector<GameObject*> spheres;
			std::vector<RayMarchSphere*> rayMarchSpheres;
			GLuint rayMarchSphereSSBO;
			int maxRayMarchSpheres;
			float timePassed = 0;
			GLuint depthBufferTex;//for depth testing after raymarch
			bool rayMarchDepthTest;
			OGLTexture* testCollisionTex;
			void InitTestCollisionTexture();
			GameObject* testCube;
			Vector3 testSphereCenter;
			float testSphereRadius;
			std::array<char, 1000 * 1000> zeros;
			GameObject* testTriangle;
			GameObject* monkey;
			void AddDebugTriangleInfoToObject(GameObject* object);
			TextureBase* metalTex;
			TextureBase* testBumpTex;
			void SendRayMarchData();
			GameObject* floor;
			GameObject* max;
			std::vector<GameObject*> walls;
			void UpdateRayMarchSpheres();

		};
	}
}

