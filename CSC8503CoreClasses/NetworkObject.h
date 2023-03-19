#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
#include "NetworkPacket.h"

namespace NCL::CSC8503 {
	class GameObject;

	class NetworkObject	: public GameObject {
	public:
		NetworkObject(int id = 0);
		virtual ~NetworkObject();

		void SetNetworkId(int id) { networkID = id; }
		
		//Called by clients
		virtual bool ReadPacket(bool delta, GamePacket& p);
		//Called by servers
		virtual bool WritePacket(GamePacket** p, int packetTp, int stateID);

		void UpdateStateHistory(int minID);

		void AddSendMessage(GamePacket* packet) { sendMessagePool.push_back(packet); }

		vector<GamePacket*> GetSendMessages() { return sendMessagePool; }

		void ClearMessagePool() {
			//Do not release the pointer
			sendMessagePool = vector<GamePacket*>();
		}

		virtual void UpdateAction(ActionPacket packet) {}
	protected:

		NetworkState& GetLatestNetworkState();

		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(NetworkState &state);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);


		NetworkState lastFullState;

		vector<GamePacket*> sendMessagePool;

		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;

		int networkID;
	};
}