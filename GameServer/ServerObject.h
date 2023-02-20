#pragma once
#include "../CSC8503CoreClasses/NetworkBase.h"
#include "../CSC8503CoreClasses/GameServer.h"
#include "./enet/enet.h"
#include <iostream>
#include <map>
#include <ctime>

using namespace NCL;
using namespace CSC8503;

#define MAX_CLIENT_NUM 4
#define CONNECT_CHECK_OFFSET 5.0f

struct ConnectInfo {
	ENetPeer* peer;
	float updateTime;

	ENetPeer* GetPeer() { return peer; }
	bool Expire(float tm) { return updateTime + CONNECT_CHECK_OFFSET > tm; }
	void UpdateTime(float tm) { updateTime = tm; }
	~ConnectInfo() {
		if (peer) {
			enet_free(peer);
		}
	}
};

class ServerObject : public GameServer, PacketReceiver
{
public:
	ServerObject(int onPort, int maxClients) : GameServer(onPort, maxClients) {}
	~ServerObject() {}

	void Init();
	void UpdateServer() override;
	void ReceivePacket(int type, GamePacket* payload, int source = -1) override;

protected:
	void CheckConnects();

	std::map<int, ConnectInfo> connects; //record time stamp 
};

