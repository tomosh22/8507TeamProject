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
			void HandlePlayerConnect(int id, NetworkState state);
			void HandlePlayerDisconnect(int id);
			void HandleUpdateState(bool delta, int id, GamePacket* payload);

			//void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

		protected:
			void UpdateToServer(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();
			std::map<int, int> stateIDs;

			GameClient* client;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::map<int, NetworkObject*> serverPlayers;
			GameObject* localPlayer;

			bool networkInit;
		};
	}
}

