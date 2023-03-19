#pragma once

namespace NCL {
	using namespace Maths;
	namespace CSC8503 {
		//class GameObject;
		class NetworkState	{
		public:
			NetworkState();
			NetworkState(Vector3 pos, Quaternion orient, int id = 0) {
				position = pos;
				orientation = orient;
				stateID = id;
			}

			virtual ~NetworkState();

			Vector3		position;
			Quaternion	orientation;
			int			stateID;
		};
	}
}

