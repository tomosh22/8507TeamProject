#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
#include "NetworkPacket.h"

namespace NCL::CSC8503 {
	class GameObject;

	class NetworkObject		{
	public:
		NetworkObject(GameObject* o, int id);
		virtual ~NetworkObject();

		//Called by clients
		virtual bool ReadPacket(bool delta, GamePacket* p);
		//Called by servers
		virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

		void UpdateStateHistory(int minID);

		GameObject* GetObjectPointer() { return object; }

	protected:

		NetworkState& GetLatestNetworkState();

		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(NetworkState &state);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);

		GameObject* object;

		NetworkState lastFullState;

		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;

		int networkID;
	};
}