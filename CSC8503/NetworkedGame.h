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
			void HandlePlayerConnect(int pid, AddObjectPacket& packet);
			void HandlePlayerDisconnect(int pid);
			void HandleUpdateState(bool delta, int pid, GamePacket* payload);
			void AddNewNetworkPlayerToWorld(int pid, NetworkState state);

		protected:
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

