#pragma once
//#include "./enet/enet.h"
struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

enum BasicNetworkMessages {
	None,
	Message,
	Connect_Confirmed,
	Player_Disconnected,
	Select_Player_Mode,
	Full_State,		//Full transform etc
	Delta_State,
	Player_Action, //received from a client
	Shutdown,

	MessageTypeMax
};

const std::string NETWORK_FILE = "network_base.txt";

static int glDefaultPort = 1234;
static std::vector<int> glIpAdress = {127, 0, 0, 1};

struct GamePacket {
	short size;
	short type;
	short networkID;

	GamePacket() {
		type		= BasicNetworkMessages::None;
		size		= 0;
		networkID = -1;
	}

	GamePacket(short type) : GamePacket() {
		this->type	= type;
	}

	int GetTotalSize() {
		return sizeof(GamePacket) + size;
	}
};

class PacketReceiver {
public:
	virtual void ReceivePacket(int type, GamePacket* payload, int source = -1) = 0;
};

class NetworkBase	{
public:
	static void Initialise();
	static void Destroy();

	static int GetDefaultPort() {
		return glDefaultPort;
	}

	static std::vector<int> GetIpAddress() {

		return glIpAdress;
	}

	void RegisterPacketHandler(int msgID, PacketReceiver* receiver) {
		packetHandlers.insert(std::make_pair(msgID, receiver));
	}
protected:
	NetworkBase();
	~NetworkBase();

	bool ProcessPacket(GamePacket* p, int peerID = -1);

	typedef std::multimap<int, PacketReceiver*>::const_iterator PacketHandlerIterator;

	bool GetPacketHandlers(int msgID, PacketHandlerIterator& first, PacketHandlerIterator& last) const {
		auto range = packetHandlers.equal_range(msgID);

		if (range.first == packetHandlers.end()) {
			return false; //no handlers for this message type!
		}
		first	= range.first;
		last	= range.second;
		return true;
	}

	_ENetHost* netHandle;

	std::multimap<int, PacketReceiver*> packetHandlers;
};