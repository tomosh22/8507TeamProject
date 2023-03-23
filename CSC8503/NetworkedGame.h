#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "NetworkObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameClient;
		class NetworkPlayer;
		const enum GameNetworkState {
			DISCONNECT,
			CONNECTING,
			CONNECTED,
		};
		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();
			void StartClient();
			void CloseClient();

			void UpdateGame(float dt) override;
			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void SetLocalPlayer(NetworkObject* object) { this->localPlayer = object; }

			int GetNetworkPlayerNum() { return serverPlayers.size() + 1; }

			void HandleConnectConfirmed(GamePacket* packet);
			void HandlePlayerDisconnect(int pid);
			void HandleSelectPlayerMode(GamePacket* payload);
			void HandleUpdateState(bool delta, int pid, GamePacket* payload);
			void AddNewNetworkPlayerToWorld(int pid, int teamID, NetworkState state);
			void HandlePlayerAction(int pid, GamePacket* payload);
			static NetworkedGame* GetInstance()
			{
				if (_instance == nullptr)
				{
					_instance = new NetworkedGame();
				}
				return _instance;
			}

			void PrintPlayerScore();
		protected:

			static NetworkedGame* _instance;
			void UpdateToServer(float dt);

			void BroadcastSnapshot();
			void UpdateMinimumState();
			std::map<int, int> stateIDs;

			GameClient* client;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::map<int, NetworkObject*> serverPlayers;
			NetworkObject* localPlayer;

			int netState = DISCONNECT;
		};
	}
}

