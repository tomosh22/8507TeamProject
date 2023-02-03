#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName)	{
	name			= objectName;
	worldID			= -1;
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
}

void GameObject::ApplyPaintAtPosition(Vector3 localPos, Vector3 halfDims, int radius, std::array<int, 200 * 200>** paintDataPtr) {
	Vector2 texCoords = (Vector2(localPos.x, localPos.z) + Vector2(halfDims.x, halfDims.z)) / (Vector2(halfDims.x, halfDims.z)*2) * Vector2(halfDims.x,halfDims.z) * 2;
	texCoords.x = round(texCoords.x);
	texCoords.y = round(texCoords.y);
	int leftS = GetLeftS(texCoords.x, radius);
	int rightS = GetRightS(texCoords.x, radius);
	int topT = GetTopT(texCoords.y, texCoords.x, radius);
	int bottomT = GetBottomT(texCoords.y, texCoords.x, radius);
	for (int s = leftS; s < rightS; s++)
	{
		for (int t = topT; t < bottomT; t++)
		{
			if ((texCoords - Vector2(s, t)).Length() < radius) {
				int index = t * (int)transform.GetScale().x + s;
				paintData->at(index) = 1;
			}
		}
	}

}

int GameObject::GetLeftS(int centerS, int radius) {
	return std::max(centerS - radius, 0);
}
int GameObject::GetRightS(int centerS, int radius) {
	return std::min(centerS + radius, (int)transform.GetScale().x-1);
}
int GameObject::GetTopT(int centerT, int centerS, int radius) {
	return std::max(centerT - radius, 0);
}
int GameObject::GetBottomT(int centerT, int centerS, int radius) {
	return std::min(centerT + radius,(int)transform.GetScale().z-1);
}


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