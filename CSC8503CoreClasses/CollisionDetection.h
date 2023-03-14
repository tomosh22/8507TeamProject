#pragma once

#include "Camera.h"

#include "Transform.h"
#include "GameObject.h"

#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "CapsuleVolume.h"
#include "Ray.h"

#define CMP(x, y) \
	(fabsf(x - y) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))
using NCL::Camera;
using namespace NCL::Maths;
using namespace NCL::CSC8503;
namespace NCL {
	class CollisionDetection
	{
	public:

		typedef struct Line {
			Vector3 start;
			Vector3 end;

			inline Line() {}
			inline Line(const Vector3& s, const Vector3& e) :
				start(s), end(e) { }
		} Line;
		typedef struct ObbPlane {
			Vector3 normal;
			float distance;

			inline ObbPlane() : normal(1, 0, 0) { }
			inline ObbPlane(const Vector3& n, float d) :
				normal(n), distance(d) { }
		} ObbPlane;
		struct Interval {
			float min;
			float max;
		};

		struct ContactPoint {
			Vector3 localA;	//where did the collision occur ...
			Vector3 localB;	//in the frame of each object !
			Vector3 normal;
			float	penetration;
		};

		struct CollisionInfo {
			GameObject* a;
			GameObject* b;
			int		framesLeft;

			ContactPoint point;

			CollisionInfo() {

			}

			void AddContactPoint(const Vector3& localA, const Vector3& localB, const Vector3& normal, float p) {
				point.localA = localA;
				point.localB = localB;
				point.normal = normal;
				point.penetration = p;
			}

			//Advanced collision detection / resolution
			bool operator < (const CollisionInfo& other) const {
				size_t otherHash = (size_t)other.a + ((size_t)other.b << 32);
				size_t thisHash = (size_t)a + ((size_t)b << 32);

				if (thisHash < otherHash) {
					return true;
				}
				return false;
			}

			bool operator ==(const CollisionInfo& other) const {
				if (other.a == a && other.b == b) {
					return true;
				}
				return false;
			}
		};


		//ray
		static Ray BuildRayFromMouse(const Camera& c);
		static bool RayBoxIntersection(const Ray& r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision);
		static bool RayIntersection(const Ray& r, GameObject& object, RayCollision& collisions);
		static bool RayAABBIntersection(const Ray& r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision);
		static bool RayOBBIntersection(const Ray& r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision);
		static bool RaySphereIntersection(const Ray& r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision);
		static bool RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision);
		static bool RayPlaneIntersection(const Ray& r, const Plane& p, RayCollision& collisions);

		static bool	AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB);

		static bool ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo);
		//AABB
		static bool AABBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
			const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		//Sphere
		static bool SphereIntersection(const SphereVolume& volumeA, const Transform& worldTransformA,
			const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		static bool AABBSphereIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
			const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		static bool AABBCapsuleIntersection(
			const CapsuleVolume& volumeA, const Transform& worldTransformA,
			const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		//Capsule
		static bool CapsuleIntersection(const CapsuleVolume& volumeA, const Transform& worldTransformA,
			const CapsuleVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		static bool SphereCapsuleIntersection(
			const CapsuleVolume& volumeA, const Transform& worldTransformA,
			const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		static bool CapsuleOBBIntersection(const CapsuleVolume& volumeA, const Transform& worldTransformA, const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		//OBB
		static bool OBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
			const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);
		static bool OBBSphereIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
			const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo);

		//OBB stuff
		static bool ClipToPlane(const ObbPlane& plane, const Line& line, Vector3* outPoint) {
			Vector3 ab = line.end - line.start;

			float nA = Vector3::Dot(plane.normal, line.start);
			float nAB = Vector3::Dot(plane.normal, ab);

			if (CMP(nAB, 0)) {
				return false;
			}

			float t = (plane.distance - nA) / nAB;
			if (t >= 0.0f && t <= 1.0f) {
				if (outPoint != 0) {
					*outPoint = line.start + ab * t;
				}
				return true;
			}

			return false;
		}

		static bool PointInOBB(const Vector3& point, const Transform& transform, const OBBVolume& obb) {
			Vector3 dir = point - transform.GetPosition();

			for (int i = 0; i < 3; ++i) {
				Matrix3 tt = Matrix3(transform.GetOrientation());
				const float* orientation = &tt.array[i][0];
				Vector3 axis(orientation[0], orientation[1], orientation[2]);

				float distance = Vector3::Dot(dir, axis);

				if (distance > obb.GetHalfDimensions()[i]) {
					return false;
				}
				if (distance < -obb.GetHalfDimensions()[i]) {
					return false;
				}
			}
			return true;
		}

		static std::vector<Line> GetEdges(const Transform& transform, const OBBVolume& obb) {
			std::vector<Line> result;
			result.reserve(12);
			std::vector<Vector3> v = GetVertices(transform, obb);

			int index[][2] = { // Indices of edges
				{ 6, 1 },{ 6, 3 },{ 6, 4 },{ 2, 7 },{ 2, 5 },{ 2, 0 },
				{ 0, 1 },{ 0, 3 },{ 7, 1 },{ 7, 4 },{ 4, 5 },{ 5, 3 }
			};

			for (int j = 0; j < 12; ++j) {
				result.push_back(Line(v[index[j][0]], v[index[j][1]]));
			}

			return result;
		}
		static std::vector<Vector3> GetVertices(const Transform& transform, const OBBVolume& obb) {
			std::vector<Vector3> v;
			v.resize(8);

			Vector3 C = transform.GetPosition();	// OBB Center
			Vector3 E = obb.GetHalfDimensions();		// OBB Extents
			//const Quaternion o = transform.GetOrientation();
			const Matrix3 t = Matrix3(transform.GetOrientation());
			Vector3 A[] = {			// OBB Axis
				Vector3(t.array[0][0], t.array[0][1], t.array[0][2]),
				Vector3(t.array[1][0], t.array[1][1], t.array[1][2]),
				Vector3(t.array[2][0], t.array[2][1], t.array[2][2])
			};

			v[0] = C + A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
			v[1] = C - A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
			v[2] = C + A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
			v[3] = C + A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
			v[4] = C - A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
			v[5] = C + A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
			v[6] = C - A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
			v[7] = C - A[0] * E[0] - A[1] * E[1] + A[2] * E[2];

			return v;
		}

		static std::vector<ObbPlane> GetPlanes(const Transform& transform, const OBBVolume& obb) {
			Vector3 c = transform.GetPosition();	// OBB Center
			Vector3 e = obb.GetHalfDimensions();		// OBB Extents
			//const Quaternion o = transform.GetOrientation();
			const Matrix3 t = Matrix3(transform.GetOrientation());
			Vector3 a[] = {			// OBB Axis
				Vector3(t.array[0][0], t.array[0][1], t.array[0][2]),
				Vector3(t.array[1][0], t.array[1][1], t.array[1][2]),
				Vector3(t.array[2][0], t.array[2][1], t.array[2][2])
			};

			std::vector<ObbPlane> result;
			result.resize(6);

			result[0] = ObbPlane(a[0], Vector3::Dot(a[0], (c + a[0] * e.x)));
			result[1] = ObbPlane(a[0] * -1.0f, -Vector3::Dot(a[0], (c - a[0] * e.x)));
			result[2] = ObbPlane(a[1], Vector3::Dot(a[1], (c + a[1] * e.y)));
			result[3] = ObbPlane(a[1] * -1.0f, -Vector3::Dot(a[1], (c - a[1] * e.y)));
			result[4] = ObbPlane(a[2], Vector3::Dot(a[2], (c + a[2] * e.z)));
			result[5] = ObbPlane(a[2] * -1.0f, -Vector3::Dot(a[2], (c - a[2] * e.z)));

			return result;
		}

		static std::vector<Vector3> ClipEdgesToOBB(const std::vector<Line>& edges, const OBBVolume& obb, const Transform& transform) {
			std::vector<Vector3> result;
			result.reserve(edges.size() * 3);
			Vector3 intersection;

			std::vector<ObbPlane> planes = GetPlanes(transform, obb);

			for (int i = 0; i < planes.size(); ++i) {
				for (int j = 0; j < edges.size(); ++j) {
					if (ClipToPlane(planes[i], edges[j], &intersection)) {
						if (PointInOBB(intersection, transform, obb)) {
							result.push_back(intersection);
						}
					}
				}
			}

			return result;
		}

		static Interval GetInterval(const Transform& transform, const OBBVolume& obb, const Vector3& axis) {
			Vector3 vertex[8];

			Vector3 C = transform.GetPosition();	// OBB Center
			Vector3 E = obb.GetHalfDimensions();		// OBB Extents
			const Quaternion o = transform.GetOrientation();
			Vector3 A[] = {			// OBB Axis
				o * Vector3(1, 0, 0),
				o * Vector3(0, 1, 0),
				o * Vector3(0, 0, 1),
			};

			vertex[0] = C + A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
			vertex[1] = C - A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
			vertex[2] = C + A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
			vertex[3] = C + A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
			vertex[4] = C - A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
			vertex[5] = C + A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
			vertex[6] = C - A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
			vertex[7] = C - A[0] * E[0] - A[1] * E[1] + A[2] * E[2];

			Interval result;
			result.min = result.max = Vector3::Dot(axis, vertex[0]);

			for (int i = 1; i < 8; ++i) {
				float projection = Vector3::Dot(axis, vertex[i]);
				result.min = (projection < result.min) ? projection : result.min;
				result.max = (projection > result.max) ? projection : result.max;
			}

			return result;
		}

		static Vector3 Unproject(const Vector3& screenPos, const Camera& cam);

		static Vector3		UnprojectScreenPosition(Vector3 position, float aspect, float fov, const Camera& c);
		static Matrix4		GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane);
		static Matrix4		GenerateInverseView(const Camera& c);

		static float PenetrationDepth(
			const OBBVolume& volumeA, const Transform& worldTransformA,
			const OBBVolume& volumeB, const Transform& worldTransformB,
			const Vector3& axis, bool* outShouldFlip
		) {
			Interval i1 = GetInterval(worldTransformA, volumeA, axis.Normalised());
			Interval i2 = GetInterval(worldTransformB, volumeB, axis.Normalised());

			if (!((i2.min <= i1.max) && (i1.min <= i2.max))) {
				return 0.0f; // No penerattion
			}

			float len1 = i1.max - i1.min;
			float len2 = i2.max - i2.min;
			float min = fminf(i1.min, i2.min);
			float max = fmaxf(i1.max, i2.max);
			float length = max - min;

			if (outShouldFlip != 0) {
				*outShouldFlip = (i2.min < i1.min);
			}

			return (len1 + len2) - length;
		}

		static Vector3 ClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& point);
	protected:

	private:
		CollisionDetection() {}
		~CollisionDetection() {}
	};
}

