#pragma once
#include "Transform.h"
#include "CollisionVolume.h"


#define TEXTURE_DENSITY 100

//typedef std::array<int, NUM_WORLD_UNITS_SQUARED * TEXTURE_DENSITY * TEXTURE_DENSITY> TEXTURE;


using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		GameObject(std::string name = "");
		~GameObject();

		virtual void Update(float dt)
		{

		}

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		void SetName(std::string newName)
		{
			name = newName;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3& outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		void SetActive(bool active)
		{
			isActive = active;
		}

		//unsigned int ssbo;
		//unsigned int texture;
		bool isPaintable;
		
		void ApplyPaintAtPosition(Vector3 localPos, Vector3 halfDims, int radius, int& startIndex, int& numInts, int& leftS, int& rightS,
			int& topT, int& bottomT, Vector2& texCoords);
		int GetLeftS(int centerS, int radius);
		int GetRightS(int centerS, int radius);
		int GetTopT(int centerT, int centerS, int radius);
		int GetBottomT(int centerT, int centerS, int radius);
	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		int			worldID;
		std::string	name;

		Vector3 broadphaseAABB;
	};
}

