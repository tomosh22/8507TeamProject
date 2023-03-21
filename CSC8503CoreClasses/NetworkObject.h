#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
#include "NetworkPacket.h"
#include <vector>

namespace NCL::CSC8503 {
	class GameObject;

	class NetworkObject	: public GameObject {
	public:
		NetworkObject(int id = 0);
		virtual ~NetworkObject();

		void SetNetworkId(int id) { networkID = id; }
		int GetNetworkId() { return networkID; }
		
		//Called by clients
		virtual bool ReadPacket(bool delta, GamePacket& p);
		//Called by servers
		virtual bool WritePacket(GamePacket** p, int packetTp, int stateID);

		void UpdateStateHistory(int minID);

		void AddSendMessage(GamePacket* packet) { sendMessagePool.push_back(packet); }

		std::vector<GamePacket*> GetSendMessages() { return sendMessagePool; }

		void ClearMessagePool() {
			//Do not release the pointer
			for (auto it : sendMessagePool) {
				if (it) {
					delete it;
				}
			}
			sendMessagePool = std::vector<GamePacket*>();
		}

		void Online() { online = true; }

		bool IsOnline() { return online; }

		void Offline() { online = false; }

		virtual void UpdateAction(ActionPacket packet) {}
	protected:

		NetworkState& GetLatestNetworkState();

		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(NetworkState &state);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);


		NetworkState lastFullState;

		std::vector<GamePacket*> sendMessagePool;

		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;

		int networkID;

		bool online = false;
	};
}