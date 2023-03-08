/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "vector3.h"
namespace NCL::Maths {
	class Plane {
	public:
		Plane(void);
		Plane(const Vector3 &normal, float distance, bool normalise = false);

		~Plane(void) {};

		//Sets the planes normal, which should be UNIT LENGTH!!!
		Plane&	SetNormal(const Vector3 &normal) { this->normal = normal; return *this;}
		//Gets the planes normal.
		Vector3 GetNormal() const { return normal; }
		//Sets the planes distance from the origin
		Plane&  SetDistance(float dist) { distance = dist; return *this; }
		//Gets the planes distance from the origin
		float	GetDistance() const { return distance; }
		//Performs a simple sphere / plane test
		bool SphereInPlane(const Vector3 &position, float radius) const;
		//Performs a simple sphere / point test
		bool PointInPlane(const Vector3 &position) const;

		float	DistanceFromPlane(const Vector3 &in) const;

		Vector3 GetPointOnPlane() const {
			return normal * -distance;
		}

		void setMidpoint(Vector3 &vertex1, Vector3& vertex2, Vector3& vertex3);

		Vector3 ProjectPointOntoPlane(const Vector3 &point) const;

		Vector3 closestPointOnLine(const Vector3& pointNotLine, const Vector3& linePoint1, const Vector3& linePoint2);

		Vector3 closestPointOnTriangleEdge(const Vector3& pNotLine, const Vector3& Point1, const Vector3& Point2, const Vector3& Point3);

		Vector3 getClosestPointOnTriangle(const Vector3& pNLine, const Vector3& P1, const Vector3& P2, const Vector3& P3);

		static Plane PlaneFromTri(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2);

	protected:
		//Unit-length plane normal
		Vector3 normal;
		//Distance from origin
		float	distance;

		Vector3 midPoint;
	};
}
