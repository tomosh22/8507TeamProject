#pragma once
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {

		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);

			virtual void UpdateServer();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;

			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
