#pragma once

#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "playerTracking.h"
#include "StateGameObject.h"
#include <array>
#include "ObjectPool.h"
#include "RenderObject.h"
#include "PropSystem.h"
#include "RespawnPoint.h"

#include "MeshAnimation.h"
#include "MeshMaterial.h"

#include <thread>
#include <mutex>

#include "RayMarchSphere.h"

namespace NCL {
	namespace CSC8503 {
		const enum GameMode {
			GAME_MODE_DEFAULT,
			GAME_MODE_GRAPHIC_TEST,
			GAME_MODE_SINGLE_GAME,
			GAME_MODE_ONLINE_GAME,
			GAME_MODE_SELECT_TEAM,
		};

		const int TWO_PLAYERS = 2;
		const int FOUR_PLAYERS = 4;

		enum TeamID {
			TEAM_DEFAULT,
			TEAM_RED,
			TEAM_BLUE,
			TEAM_GREEN,
			TEAM_YELLOW,
		};

		class Projectile;
		class TutorialGame {
		public:
			TutorialGame(GameWorld* gameWorld);
			~TutorialGame();

			virtual void UpdateGame(float dt);
			int SelectTeam();
			//void InitWorld(); //moved from protected
			void InitGraphicTest();
			void InitSingleGameMode();
			void InitOnlineGame(int treamID);
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
			TextureBase* wallTex = nullptr;
			ShaderBase* basicShader = nullptr;


			//Coursework Meshes
			MeshGeometry* charMesh = nullptr;
			MeshGeometry* enemyMesh = nullptr;
			MeshGeometry* bonusMesh = nullptr;

			MeshGeometry* playerMesh = nullptr;

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
			//void InitGameObjects();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitMixedGridWorldtest(int numRows, int numCols, float rowSpacing, float colSpacing);
			
		

			void InitDefaultFloor();
			void InitDefaultFloorRunway();

			void InitGameExamples();

			void UpdateWorldCamera(float dt);
			void CameraLockOnPlayer();
			void RayCast();
			void LockedObjectMovement();
			void ControlPlayer(float dt);
			void movePlayer(playerTracking* unitGoat);
			void setLockedObject(GameObject* goatPlayer);

			

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
			GameObject* AddWallToWorld2(const Vector3& position, Vector3 dimensions); 
			GameObject* AddLadderToWorld(const Vector3& position, float height, bool rotated);

			playerTracking* AddPlayerToWorld(const Vector3& position, Quaternion& orientation, int team = 0, RespawnPoint* rp = nullptr);
			GameObject* AddEnemyGoatToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			//this was me
			GameObject* AddDebugTriangleToWorld(const Vector3& position);

			void AddMapToWorld();
			void AddMapToWorld2();
			void AddStructureToWorld();
			void AddTowersToWorld();
			void AddPlatformsToWorld();
			void AddPowerUps(); 
			void AddRespawnPoints();

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else

			GameTechRenderer* renderer = nullptr;
#endif
			GameWorld* world = nullptr;
			PhysicsSystem* physics = nullptr;
			PropSystem* propSystem = nullptr;

			bool useGravity;

			float		forceMagnitude;

			Vector3 viewOffset = Vector3(10.0f, 3.0f, 10.0f);

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

			//playerTracking* goatCharacter = nullptr;
			GameObject* EnemyGoat = nullptr;
			playerTracking* playerObject = nullptr;

			const int bulletLifeLimit = 2;
			const int bulletDeletionLimit = 5;
			float runnigBulletTime = 0.0f;
			float BulletDeleteTime = 0.0f;

			ObjectPool<Projectile>* objectpool;

			
			
			//this was me
			OGLComputeShader* computeShader;
			void RunComputeShader(GameObject* floor,int width, int height, int leftS, int rightS, int topT, int bottomT, int radius,Vector2 center, int teamID);
			
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

			std::vector<GameObject*> spheres;
			float timePassed = 0;
			GLuint depthBufferTex;//for depth testing after raymarch
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
			GameObject* floor;
			GameObject* max;
			std::vector<GameObject*> walls;

			int gameMode = GAME_MODE_DEFAULT;

			GLuint tempSSBO;

			
			//Team currentTeam = Team::team2;
			//int currentTeamInt = 1;
			
			int highestTriCount = 0;

			

			GameObject* testSphere0 = nullptr;
			GameObject* testSphere1 = nullptr;
			GameObject* testSphere2 = nullptr;
			GameObject* testSphere3 = nullptr;
			GameObject* testSphere4 = nullptr;

			//Player Animation 
			OGLShader* characterShader;
			MeshMaterial* playerMaterial;
			int currentFrame;
			float frameTime;
			void UpdateAnimations(float dt);
			std::vector<RenderObject*> animatedObjects;

			bool pause = false;

			int playerNum = 0;
			

		};

		/*

		Each of the little demo scenarios used in the game uses the same 2 meshes,
		and the same texture and shader. There's no need to ever load in anything else
		for this module, even in the coursework, but you can add it if you like!

		*/

	}
}


