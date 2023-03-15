#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include"playerTracking.h"


#include "StateGameObject.h"
#include <array>
#include"ObjectPool.h"
#include "RenderObject.h"
#include "PropSystem.h"

namespace NCL {
	namespace CSC8503 {
		const int GAME_MODE_DEFAULT = 0;
		const int GAME_MODE_GRAPHIC_TEST = 1;
		const int GAME_MODE_PHISICAL_TEST = 2;
		class Projectile;
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			void SelectMode();
			//void InitWorld(); //moved from protected
			void InitGraphicTest();
			void InitPhysicalTest();
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


			void DispatchComputeShaderForEachTriangle(GameObject* object, Vector3 spherePosition, float sphereRadius, int teamID, bool clearMask = false);

			MeshGeometry* capsuleMesh = nullptr;
			MeshGeometry* cubeMesh = nullptr;
			MeshGeometry* sphereMesh = nullptr;

			//this was me
			MeshGeometry* triangleMesh = nullptr;
			MeshGeometry* monkeyMesh = nullptr;
			MeshGeometry* floorMesh = nullptr;
			MeshGeometry* maxMesh = nullptr;
			MeshGeometry* basicWallMesh = nullptr;
			MeshGeometry* bunnyMesh = nullptr;

			TextureBase* basicTex = nullptr;
			ShaderBase* basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry* charMesh = nullptr;
			MeshGeometry* enemyMesh = nullptr;
			MeshGeometry* bonusMesh = nullptr;

			MeshGeometry* playerMesh = nullptr;

			enum Team {
				teamNull,
				team1,
				team2,
				team3,
				team4,
				team5,
				team6,
				team7,
				team8,
			};

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

			class RayMarchSphere : public GameObject {
			public:
				Vector3 center;
				float radius;
				Vector3 color;
			};

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& scale, bool rotated = false);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, bool render, float inverseMass = 10.0f, bool physics = true);
			GameObject* AddRayMarchSphereToWorld(const Vector3& position, float radius);
			

			
			GameObject* AddRunwayToWorld(const Vector3& position);
			
			Projectile* AddBulletToWorld(playerTracking* playableCharacter);
			Projectile* useNewBullet(playerTracking* passedPlayableCharacter);

			
			Projectile* FireBullet(playerTracking* selectedPlayerCharacter);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 1.0f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inversMass = 1.0f);
			

			GameObject* AddMonkeyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, bool physics = true);
			GameObject* AddMaxToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddBunnyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, bool physics = true);
			GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			playerTracking* AddPlayerToWorld(const Vector3& position, Quaternion& orientation);
			GameObject* AddEnemyGoatToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			//this was me
			GameObject* AddDebugTriangleToWorld(const Vector3& position);

			void AddMapToWorld();

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else

			GameTechRenderer* renderer;
#endif

			PhysicsSystem*		physics;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;
			GameObject* phantomCubeOutput = nullptr;



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
			playerTracking* testPlayer = nullptr;

			const int bulletLifeLimit = 2;
			const int bulletDeletionLimit = 5;
			float runnigBulletTime = 0.0f;
			float BulletDeleteTime = 0.0f;

			ObjectPool<Projectile>* objectpool;

			
			
			//this was me
			OGLComputeShader* computeShader;
			void RunComputeShader(GameObject* floor,int width, int height, int leftS, int rightS, int topT, int bottomT, int radius,Vector2 center, int teamID);
			OGLShader* quadShader;
			
			void InitQuadTexture();
			TextureBase* floorTex = nullptr;
			void InitPaintableTextureOnObject(GameObject* object, bool rotated = false);


			

			GLuint triangleSSBO;
			GLuint debugTriangleSSBO;
			void SetUpTriangleSSBOAndDataTexture();
			OGLComputeShader* triComputeShader;
			OGLComputeShader* triRasteriseShader;
			//OGLTexture* triDataTex;//1d texture
			GLuint triangleBoolSSBO;
			GLuint triangleRasteriseSSBO;
			GLuint triangleRasteriseSSBOSecondShader;
			bool SphereTriangleIntersection(Vector3 sphereCenter, float sphereRadius, Vector3 v0, Vector3 v1, Vector3 v2, Vector3& intersectionPoint);

			void DispatchComputeShaderForEachPixel();
			OGLComputeShader* rayMarchComputeShader;
			int maxSteps;
			float hitDistance;
			float noHitDistance;
			float debugValue;

			

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
			GameObject* bunny;
			void AddDebugTriangleInfoToObject(GameObject* object);
			
			TextureBase* metalTex;
			TextureBase* testBumpTex;
			void SendRayMarchData();
			GameObject* floor;
			GameObject* max;
			std::vector<GameObject*> walls;
			void UpdateRayMarchSpheres();

			int gameMode = GAME_MODE_DEFAULT;

			GLuint tempSSBO;

			
			//Team currentTeam = Team::team2;
			//int currentTeamInt = 1;
			
			int highestTriCount = 0;

			TextureBase* ironDiffuse = nullptr;
			TextureBase* ironBump = nullptr;
			TextureBase* ironMetallic = nullptr;
			TextureBase* ironRoughness = nullptr;

			TextureBase* crystalDiffuse = nullptr;
			TextureBase* crystalBump = nullptr;
			TextureBase* crystalMetallic = nullptr;
			TextureBase* crystalRoughness = nullptr;
			TextureBase* crystalHeightMap = nullptr;
			TextureBase* crystalEmissionMap = nullptr;
			TextureBase* crystalAOMap = nullptr;
			TextureBase* crystalOpacityMap = nullptr;
			TextureBase* crystalGlossMap = nullptr;

			TextureBase* spaceShipDiffuse = nullptr;
			TextureBase* spaceShipBump = nullptr;
			TextureBase* spaceShipMetallic = nullptr;
			TextureBase* spaceShipRoughness = nullptr;
			TextureBase* spaceShipHeightMap = nullptr;
			TextureBase* spaceShipEmissionMap = nullptr;
			TextureBase* spaceShipAOMap = nullptr;
			TextureBase* spaceShipOpacityMap = nullptr;
			TextureBase* spaceShipGlossMap = nullptr;

			PBRTextures* crystalPBR;
			PBRTextures* spaceShipPBR;
			PBRTextures* rockPBR;
			PBRTextures* grassWithWaterPBR;
			PBRTextures* fencePBR;
			

			GameObject* testSphere0 = nullptr;
			GameObject* testSphere1 = nullptr;
			GameObject* testSphere2 = nullptr;
			GameObject* testSphere3 = nullptr;
			GameObject* testSphere4 = nullptr;

			bool rayMarch = true;

			

		};
	}
}


