#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "NetworkObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameClient;
		class NetworkPlayer;

		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();
			void StartClient(char a, char b, char c, char d);
			void CloseClient();

			void UpdateGame(float dt) override;
			void ReceivePacket(int type, GamePacket* payload, int source) override;
			void SetLocalPlayer(NetworkObject* object) { this->localPlayer = object; }

			void HandleConnectConfirmed();
			void HandlePlayerDisconnect(int pid);
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
		protected:

			static NetworkedGame* _instance;
			void UpdateToServer(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();
			std::map<int, int> stateIDs;

			GameClient* client;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::map<int, NetworkObject*> serverPlayers;
			NetworkObject* localPlayer;

			bool online = false;
		};
	}
}

