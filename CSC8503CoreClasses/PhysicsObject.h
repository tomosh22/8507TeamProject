#pragma once
#include "Vector3.h"
#include "Matrix4.h"
#include "Matrix3.h"
#include "Quaternion.h"
using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8503 {
		class Transform;

		class PhysicsObject	{
		public:

			inline static const Vector3 gravityDirection{ 0,-1,0 };

			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}

			Vector3 GetTorque() const {
				return torque;
			}

			Vector3 GetForce() const {
				return force;
			}

			bool getAlphaPhasing() {
				return alphaPhasing;
			}

			bool getAffectedByGravity() {
				return AffectedByGravity;
			}

			void SetInverseMass(float invMass) {
				inverseMass = invMass;
			}

			float GetInverseMass() const {
				return inverseMass;
			}

			bool GetFloorContact() {
				return floorcontact;
			}

			void setFloorContactTrue() {
				floorcontact = true;
			}

			void setFloorContactFalse() {
				floorcontact = false;
			}

			bool GetCanJump() {
				return canJump;
			}

			void setCanJumpTrue() {
				canJump = true;
				return;
			}

			void setCanJumpFalse() {
				canJump = false;
				return;
			}


			void toggleCanJump() {
				if (!canJump) {
					canJump = true;
					return;
				}
				canJump = false;
				return;
			}


			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);

			void setCoeficient(float newCoe) {
				coeficient = newCoe;
			}

			float GetCoeficient() {
				return coeficient;
			}

			void ClearForces();


			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			void SetAffectedByGravityFalse() {
				AffectedByGravity = false;
			}

			void SetLayerID() {
				layerID = 2;
			}

			int getLayerId()const {
				return layerID;
			}

			void InitCubeInertia();
			void InitSphereInertia();

			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return inverseInteriaTensor;
			}

			void setTorqueFriction(float newTorqueFriction) {
				torqueFriction = std::clamp(newTorqueFriction, 0.0f, 0.01f);
			}

			float getTorqueFriction() {
				return torqueFriction;
			}

			//this was me
			static Vector3 WorldSpaceToBarycentricCoords(Vector3 point, Vector3 vertA, Vector3 vertB, Vector3 vertC);
			static float GetAreaFromTriangleVerts(Vector3 a, Vector3 b, Vector3 c);

		protected:
			const CollisionVolume* volume;
			Transform*		transform;

			float torqueFriction;
			float coeficient = 0.33f;
			float inverseMass;
			float elasticity;
			float friction;
			int layerID = 0;
			bool floorcontact;
			bool canJump;
			bool alphaPhasing = false;
			bool AffectedByGravity = true;
			

			//linear stuff
			Vector3 linearVelocity;
			Vector3 force;
			
			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInteriaTensor;
		};
	}
}


