#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include"playerTracking.h"
#include "StateGameObject.h"
#include"Projectile.h"
#include <array>
#include"ObjectPool.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			//void InitWorld(); //moved from protected
			void InitWorldtest();
			void InitWorldtest2();

			void setLockedObjectNull();

			void addToRunningBulletTime(float timeIncrement) {
				runnigBulletTime += timeIncrement;
			}

			void resetRunningBulletTime() {
				runnigBulletTime = 0.0f;
			}

			bool runningBulletTimeLimitTest(float rateFireLim) {
				if (runnigBulletTime >= rateFireLim) {
					resetRunningBulletTime();
					return true;
				}
				return false;
			}

			void setEnemyGoat(GameObject* assignCharcter);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void setGoatCharacter(playerTracking* assignCharcter);

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitMixedGridWorldtest(int numRows, int numCols, float rowSpacing, float colSpacing);
			
		

			void InitDefaultFloor();
			void InitDefaultFloorRunway();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void movePlayer(playerTracking* unitGoat);
			void setLockedObject(GameObject* goatPlayer);

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddRunwayToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			Projectile* AddBulletToWorld(playerTracking* playableCharacter);
			Projectile* useNewBullet(playerTracking* passedPlayableCharacter);

			
			Projectile* FireBullet(playerTracking* selectedPlayerCharacter);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddDestructableCubeToWorld(const Vector3& position, Vector3 dimensions, float dt, float inverseMass = 10.0f);
			GameObject* AddMonkeyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMaxToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			playerTracking* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyGoatToWorld(const Vector3& position);
			GameObject* AddPlayerToWorld(const Vector3& position, Quaternion& orientation);
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
			GameObject* phantomCubeOutput = nullptr;

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
			//Vector3 lockedOffset		= Vector3(0, 14, 20); - origonal
			Vector3 lockedOffset = Vector3(0, 7, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			StateGameObject* AddStateObjectToWorld(const Vector3& position); //for use in constraints 
			StateGameObject* testStateObject; //for use in constraints 

			GameObject* objClosest = nullptr;

			playerTracking* goatCharacter = nullptr;
			GameObject* EnemyGoat = nullptr;
			playerTracking* testplayer = nullptr;

			const int bulletLifeLimit = 2;
			const int bulletDeletionLimit = 5;
			float runnigBulletTime = 0.0f;
			float BulletDeleteTime = 0.0f;

			ObjectPool<Projectile>* objectpool;

			GameObject* worldFloor;
			
			//this was me
			OGLComputeShader* computeShader;
			void RunComputeShader(GameObject* floor,int width, int height, int leftS, int rightS, int topT, int bottomT, int radius,Vector2 center, int teamID);
			OGLShader* quadShader;
			TextureBase* quadTex = nullptr;
			void InitQuadTexture();
			TextureBase* floorTex = nullptr;
			void InitPaintableTextureOnObject(GameObject* object);

			void DispatchComputeShaderForEachTriangle(GameObject* object);
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
			struct RayMarchSphere {
				Vector3 center;
				float radius;
				Vector3 color;
			};
			std::vector<GameObject*> spheres;
			std::vector<RayMarchSphere> rayMarchSpheres;
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

		};
	}
}


