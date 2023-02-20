#pragma once
#include "NetworkBase.h"
#include "map"

namespace NCL {
	namespace CSC8503 {

#define MAX_CLIENT_NUM 4

		class GameServer : public NetworkBase{
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			bool SendGlobalPacket(GamePacket& packet);
			bool SendGlobalPacket(int msgID);

			virtual void UpdateServer();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;

			int incomingDataRate;
			int outgoingDataRate;

			std::map<int, float> connects;
		};
	}
}
