#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#define TEXTURE_DENSITY 4

//typedef std::array<int, NUM_WORLD_UNITS_SQUARED * TEXTURE_DENSITY * TEXTURE_DENSITY> TEXTURE;



namespace NCL::CSC8503 {

	enum LayerMask
	{
		Default,
		Bullet,
		Trigger, 
	};


	class NetworkObject;
	class RenderObject;
	class PhysicsObject;
	class AgentMovement;
	class ContactPoint;
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

		virtual void Update(float dt)
		{

		}



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


		void SetName(std::string name)  {
			this->name = name;
		}

		int GetTeamId()
		{
			return teamID;
		}

		void SetTeamId(int team)
		{
			teamID = team;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject)
		{
			
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		bool GetIsAlpha() {
			return isAlpha;
		}

		void setIsAlpha() {
			isAlpha = true;
		}

		void UpdateBroadphaseAABB();


		void SetWorldID(int newID) {
			worldID = newID;
		}

		void setImpactAbsorbtionAmount(float newAbsorbtionAmount) {
			impactAbsorbtionAmount = newAbsorbtionAmount;
		}

		float getImpactAbsorbtionAmount() {
			return impactAbsorbtionAmount;
		}

		int		GetWorldID() const {
			return worldID;
		}

		void SetLayerMask(LayerMask layerMask)
		{
			layer = layerMask;
		}

		LayerMask GetLayerMask()
		{
			return layer;
		}

		virtual float collisionInfo() {
			return 0;
		}

		virtual std::string id()
		{
		    return "Object";
	    }

		//unsigned int ssbo;
		//unsigned int texture;
		bool isPaintable;

		/*void ApplyPaintAtPosition(Vector3 localPos, Vector3 halfDims, int radius, int& startIndex, int& numInts, int& leftS, int& rightS,
			int& topT, int& bottomT, Vector2& texCoords);
		int GetLeftS(int centerS, int radius);
		int GetRightS(int centerS, int radius);
		int GetTopT(int centerT, int centerS, int radius);
		int GetBottomT(int centerT, int centerS, int radius);*/

	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;
		AgentMovement*      agentObject;
		LayerMask           layer;

		bool        isAlpha = false;
		bool		isActive;
		int			worldID;
		int			teamID;
		float impactAbsorbtionAmount;
		std::string	name;

		Vector3 broadphaseAABB;
	};
}


