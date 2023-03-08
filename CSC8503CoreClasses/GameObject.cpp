#include "GameObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "CollisionDetection.h"
#define DB_PERLIN_IMPL
//#include "PerlinNoise.hpp"

using namespace NCL::CSC8503;


GameObject::GameObject(string objectName)	{
	name			= objectName;
	worldID			= -1;
	impactAbsorbtionAmount = 0.0f;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
	delete networkObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
	}
	

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}

/*void GameObject::ApplyPaintAtPosition(Vector3 localPos, Vector3 halfDims, int radius, int& startIndex, int& numInts, int& leftS, int& rightS,
		int& topT, int& bottomT, Vector2 & texCoords){
		Vector3 newLocalPos = localPos * TEXTURE_DENSITY;
		Vector3 newHalfDims = halfDims * TEXTURE_DENSITY;
		int newRadius = radius * TEXTURE_DENSITY;
		texCoords = (Vector2(newLocalPos.x, newLocalPos.z) + Vector2(newHalfDims.x, newHalfDims.z)) / (Vector2(newHalfDims.x, newHalfDims.z) * 2) * Vector2(newHalfDims.x, newHalfDims.z) * 2;
		texCoords.x = round(texCoords.x);
		texCoords.y = round(texCoords.y);

		leftS = GetLeftS(texCoords.x, newRadius);
		rightS = GetRightS(texCoords.x, newRadius);
		topT = GetTopT(texCoords.y, texCoords.x, newRadius);
		bottomT = GetBottomT(texCoords.y, texCoords.x, newRadius);

		startIndex = topT * (int)transform.GetScale().x * TEXTURE_DENSITY + leftS;
		int endIndex = bottomT * (int)transform.GetScale().x * TEXTURE_DENSITY + rightS;
		numInts = endIndex - startIndex;
		return;


	}

	int GameObject::GetLeftS(int centerS, int radius) {
		return std::max(centerS - radius, 0);
	}
	int GameObject::GetRightS(int centerS, int radius) {
		return std::min(centerS + radius, (int)transform.GetScale().x * TEXTURE_DENSITY - 1);
	}
	int GameObject::GetTopT(int centerT, int centerS, int radius) {
		return std::max(centerT - radius, 0);
	}
	int GameObject::GetBottomT(int centerT, int centerS, int radius) {
		return std::min(centerT + radius, (int)transform.GetScale().z * TEXTURE_DENSITY - 1);
	}*/




//int GameObject::GetLeftS(int centerS, int radius) {
//	return std::max(centerS - radius, centerS * (int)transform.GetScale().x);
//}
//int GameObject::GetRightS(int centerS, int radius) {
//	return std::min(centerS + radius, centerS * (int)transform.GetScale().x + (int)transform.GetScale().x - 1);
//}
//int GameObject::GetTopT(int centerT, int centerS, int radius) {
//	return std::max(centerT - radius * (int)transform.GetScale().x, centerS);
//}
//int GameObject::GetBottomT(int centerT, int centerS, int radius) {
//	return std::min(centerT + radius * (int)transform.GetScale().x
//		, ((int)transform.GetScale().x * (int)transform.GetScale().z - 1) - (((int)transform.GetScale().x - 1) - centerS));
//}
