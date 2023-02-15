#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include"playerTracking.h"
#include "StateGameObject.h"
#include"Projectile.h"
//#include "C:\Users\c2065963\source\repos\Advanced compter physics\Build\CSC8503\TimeKeeper.h"

#include "ObjectPool.h"

namespace NCL {
	namespace CSC8503 {
		class AgentMovement;
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual int UpdateGame(float dt); //testing changing update game to int return "had to try it :)"
			//Timekeeper         runningtime;
			void UpdateGameRenderer(float dt);


			void InitWorld(); //moved from protected
			void InitWorldtest();
			void InitWorldtest2();

			/*void updateBehaviourTree(BehaviourSequence* root1);*/
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

			void addToBulletDeleteTime(float timeIncrement) {
				BulletDeleteTime += timeIncrement;
			}
			
			void resetBulletDeleteTime() {
				BulletDeleteTime = 0.0f;
			}

			bool bulletDeleteTimeTest() {
				if (BulletDeleteTime >= bulletDeletionLimit) {
					resetBulletDeleteTime();
					return true;
				}
				return false;
			}

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void BridgeConstraintTest();
			void chainBallTest();

			void setGoatCharacter(playerTracking* assignCharcter);
			void setEnemyGoat(GameObject* assignCharcter);

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitMixedGridWorldtest(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitMixedGridWorldGoatStrip(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();
			void InitDefaultFloorRunway();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void movePlayer(playerTracking* unitGoat);
			void ResetPhantomCube(GameObject* cubeToReset);
			void setLockedObject(GameObject* goatPlayer);

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddRunwayToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			Projectile* AddBulletToWorld(playerTracking* playableCharacter);
			Projectile* useNewBullet(playerTracking* passedPlayableCharacter);
			
			Projectile* useRecycledBulet(playerTracking* passedPlayableCharacter);
			Projectile* FireBullet(playerTracking* selectedPlayerCharacter);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddDestructableCubeToWorld(const Vector3& position, Vector3 dimensions, float dt,float inverseMass = 10.0f);
			GameObject* AddPhantomCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddPhantomBoxToWorld(const Vector3& position, Vector3 dimensions, float inverseMass);

			playerTracking* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyGoatToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);



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

			//BehaviourSequence* newrootSequence = nullptr;

			GameObject* selectionObject = nullptr;
			GameObject* phantomCubeOutput = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;

			TextureBase*	basicTex	= nullptr;
			ShaderBase*		basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 7, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}


			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			AgentMovement* amovement = nullptr;
			StateGameObject* testStateObject;
			GameObject* objClosest = nullptr;
			playerTracking* goatCharacter = nullptr;
			GameObject* EnemyGoat = nullptr;
			playerTracking* testplayer = nullptr;

			const int bulletLifeLimit = 2;
			const int bulletDeletionLimit = 5;
			float runnigBulletTime = 0.0f;
			float BulletDeleteTime = 0.0f;

			ObjectPool<Projectile> * objectpool;

			/*OGLComputeShader* computeShader;
			void RunComputeShader(GameObject* floor, int width, int height, int leftS, int rightS, int topT, int bottomT, int radius, Vector2 center);
			OGLShader* quadShader;
			OGLTexture* quadTex = nullptr;
			void InitQuadTexture();
			TextureBase* floorTex = nullptr;
			void InitPaintableTextureOnObject(GameObject* object);*/
		};
	}
}
