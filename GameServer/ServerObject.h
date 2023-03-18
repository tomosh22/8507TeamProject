#pragma once
#include "../CSC8503CoreClasses/NetworkBase.h"
#include "../CSC8503CoreClasses/GameServer.h"
#include "../CSC8503CoreClasses/NetworkPacket.h"
#include "./enet/enet.h"
#include <iostream>
#include <map>
#include <ctime>

namespace NCL {
	namespace CSC8503 {

		const int MAX_CLIENT_NUM = 4;
		const int CONNECT_CHECK_OFFSET = 10;

		struct ConnectInfo {
			ENetPeer* peer;
			time_t updateTime;

			ENetPeer* GetPeer() { return peer; }
			bool Expire(time_t tm) { return updateTime + CONNECT_CHECK_OFFSET <= tm; }
			void UpdateTime(time_t tm) { updateTime = tm; }
		};

		class ServerObject : public GameServer, PacketReceiver
		{
		public:
			ServerObject(int onPort, int maxClients) : GameServer(onPort, maxClients) {}
			~ServerObject() {}

			void Init();
			void UpdateServer() override;
			void ReceivePacket(int type, GamePacket* payload, int source = -1) override;
			void ReplyToClient(int type, ENetPeer* peer);
			void Broadcast(int type, ENetPacket* msg, int source);
			//handle event
			void HandleConnected(ENetPeer* peer);
			void HandleDisconnected(int pid);
			void HandleReceived(int pid, GamePacket* packet);

			GamePacket* DeepCopyPacket(GamePacket* packet);

		protected:
			void CheckConnectExpire();

			std::map<int, ConnectInfo> connects; //record time stamp 
		};
	}
}

