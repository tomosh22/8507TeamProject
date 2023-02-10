#pragma once

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);

			Vector3 GetPosition() const {
				return position;
			}

			Vector3 GetScale() const {
				return scale;
			}

			Quaternion GetOrientation() const {
				return orientation;
			}

			Matrix4 GetMatrix() const {
				return matrix;
			}

			Vector3 GetForward()
			{
				return (orientation * Vector3(0, 0, -1)).Normalised();
			}

			void Rotate(Vector3 angle)
			{
				Vector3 selfAngle = GetOrientation().ToEuler();
				SetOrientation(Quaternion::EulerAnglesToQuaternion(selfAngle.x + angle.x, selfAngle.y + angle.y, selfAngle.z + angle.z));
			}

			void RotateTo(Vector3 angle)
			{			
				SetOrientation(Quaternion::EulerAnglesToQuaternion( angle.x,  angle.y, angle.z));
			}

			void UpdateMatrix();
		protected:
			Matrix4		matrix;
			Quaternion	orientation;
			Vector3		position;

			Vector3		scale;
		};
	}
}

