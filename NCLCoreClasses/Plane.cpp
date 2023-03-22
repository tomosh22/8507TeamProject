/*
Part of Newcastle University's Game Engineering source code.
Use as you see fit!
Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Plane.h"
using namespace NCL;
using namespace NCL::Maths;

Plane::Plane(void) {
	normal = Vector3(0, 1, 0);
	midPoint = Vector3(0, 0, 0);
	distance = 0.0f;
};

Plane::Plane(const Vector3& normal, float distance, bool normalise) {
	if (normalise) {
		float length = normal.Length();
		this->normal = normal;
		this->normal.Normalise();
		this->distance = distance / length;
	}
	else {
		this->normal = normal;
		this->distance = distance;
	}
}
bool Plane::SphereInPlane(const Vector3& position, float radius) const {
	if (Vector3::Dot(position, normal) + distance <= -radius) {
		return false;
	}
	return true;
}
bool Plane::PointInPlane(const Vector3& position) const {
	if (Vector3::Dot(position, normal) + distance < -0.001f) {
		return false;
	}
	return true;
}
Plane Plane::PlaneFromTri(const Vector3& v0, const Vector3& v1, const Vector3& v2) {
	Vector3 v1v0 = v1 - v0;
	Vector3 v2v0 = v2 - v0;
	Vector3 normal = Vector3::Cross(v1v0, v2v0);

	normal.Normalise();
	float d = -Vector3::Dot(v0, normal);
	return Plane(normal, d, false);
}
float	Plane::DistanceFromPlane(const Vector3& in) const {
	return Vector3::Dot(in, normal) + distance;
}
Vector3 Plane::ProjectPointOntoPlane(const Vector3& point) const {
	float distance = DistanceFromPlane(point);

	return point - (normal * distance);
}


void Plane::setMidpoint(Vector3& vertex1, Vector3& vertex2, Vector3& vertex3) {
	Vector3 mid;
	mid.x = (vertex1.x + vertex2.x + vertex3.x) / 3.0;
	mid.y = (vertex1.y + vertex2.y + vertex3.y) / 3.0;
	mid.z = (vertex1.z + vertex2.z + vertex3.z) / 3.0;
	midPoint = mid;
}

Vector3 Plane::GetIntersection(const Vector3& pos, const Vector3& vec) {
	double dotValue = Vector3::DoubleDot(vec, normal); 

	//If the dot product results in 0, the vectors are parallel to the plane and have no intersection
	if (dotValue == 0) {
		return Vector3();
	}
	double dotPN = Vector3::DoubleDot(pos, normal);
	double t = abs((distance - dotPN) / dotValue);
	Vector3 intersection = pos + vec * static_cast<float>(t);
	return intersection;
}

Plane Plane::PlaneFromVector(const Vector3& vec1, const Vector3& vec2) {
	Plane plane;
	plane.normal = Vector3::Cross(vec1, vec2);
	plane.distance = -Vector3::Dot(vec1, vec2);
	return plane;
}

Plane Plane::PlaneFromTri(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2) {
	Vector3 v1v0 = v1-v0;
	Vector3 v2v0 = v2-v0;

	return closestPointOnLine;
}


Vector3 Plane::closestPointOnTriangleEdge(const Vector3& pNotLine, const Vector3& Point1, const Vector3& Point2, const Vector3& Point3) {
	float distToPoint1 = closestPointOnLine(pNotLine, Point1, Point2).Length();
	float distToPoint2 = closestPointOnLine(pNotLine, Point1, Point3).Length();
	float distToPoint3 = closestPointOnLine(pNotLine, Point2, Point3).Length();
	if (distToPoint1 <= distToPoint2 && distToPoint1 <= distToPoint3) {
		return closestPointOnLine(pNotLine, Point1, Point2);
	}
	else if (distToPoint2 <= distToPoint1 && distToPoint2 <= distToPoint3) {
		return closestPointOnLine(pNotLine, Point1, Point3);
	}

	else {
		return closestPointOnLine(pNotLine, Point2, Point3);
	}
}

Vector3 Plane::getClosestPointOnTriangle(const Vector3& pNLine, const Vector3& P1, const Vector3& P2, const Vector3& P3) {
	Vector3 pointOnTriangleEdge = closestPointOnTriangleEdge(pNLine, P1, P2, P3);
	Vector3 pointOnTrianglePlane = ProjectPointOntoPlane(pNLine);
	if ((midPoint - pointOnTriangleEdge).Length() <= (midPoint - pointOnTrianglePlane).Length()) {
		return pointOnTriangleEdge;
	}
	else
	{
		return pointOnTrianglePlane;
	}

}