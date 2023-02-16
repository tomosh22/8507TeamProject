#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

using std::vector;

namespace NCL::CSC8503 {

	class NetworkObject;
	class RenderObject;
	class PhysicsObject;
	class AgentMovement;
	class GameObject	{
	public:
		GameObject(std::string name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}
		// testing ignoring detection
		void toggleobjectdetection() {
			if (!isActive) {
				isActive = true;
			}
			isActive = false;
		}
		// end ignoring detection

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		void SetActive(bool active) {
			this->isActive = active;
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

		AgentMovement* GetAgentObject()const {
			return agentObject;
		}

		void SetAgentObject(AgentMovement* newObject) {
			agentObject = newObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		void resetPhysicsObject() {
			physicsObject = nullptr;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject)
		{
			
		}

		virtual void OnCollisionEnd(GameObject* otherObject) 
		{
			
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		bool GetIsAlpha(){
			return isAlpha;
		}

		void setIsAlpha() {
			isAlpha = true;
		}

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		void setImpactAbsorbtionAmount(float newAbsorbtionAmount) {
			impactAbsorbtionAmount = newAbsorbtionAmount;
		}

		float getImpactAbsorbtionAmount() {
			return impactAbsorbtionAmount;
		}

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
		AgentMovement*      agentObject;


		bool        isAlpha = false;
	    bool		isActive;
		int			worldID;
	    float impactAbsorbtionAmount;
		std::string	name;

		Vector3 broadphaseAABB;
	};
}

