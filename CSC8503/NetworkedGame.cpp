#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;
	string message;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame()	{
	client = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
}

NetworkedGame::~NetworkedGame()	{
	if (client) {
		delete client;
	}
}

void NetworkedGame::StartClient(char a, char b, char c, char d) {
	client = new GameClient();
	if (!client->Connect(a, b, c, d, NetworkBase::GetDefaultPort())) { 
		std::cout << "Client:: connect server flase, ip: " << a << "." << b << "." << c << "." << d << " port: " << NetworkBase::GetDefaultPort() << std::endl;
		return; 
	}

	for (int i = BasicNetworkMessages::None; i < BasicNetworkMessages::MessageTypeMax; i++){
		client->RegisterPacketHandler(i, this);
	}
	std::cout << "Client: connect server success" << std::endl;
	networkInit = true;
}

void NetworkedGame::CloseClient() {
	networkInit = false;
	if (client) {
		delete client;
	}
	for (auto i : serverPlayers) {
		if (i.second) {
			delete i.second;
		}
		serverPlayers.erase(i.first);
	}
}

void NetworkedGame::UpdateGame(float dt) {
	if (!client && Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		StartClient(127, 0, 0, 1);
	}
	if (networkInit) {
		//broadcast
		timeToNextPacket -= dt;
		if (timeToNextPacket < 0) {
			UpdateToServer(dt);
			timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
		}
		//update from server
		client->UpdateClient();
	}

	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdateToServer(float dt) {
	ClientPacket newPacket;

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
		newPacket.buttonstates[0] = 1;
		newPacket.lastID = 0;
		client->SendPacket(newPacket);
	}
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		int playerState = 0;
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			client->SendPacket(*newPacket);
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = min(minID, i.second);
		maxID = max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	switch (type) {
	case Player_Connected:
	{
		ConnectPacket* packet = (ConnectPacket*)payload;
		HandlePlayerConnect(source, packet->fullState);
		std::cout << "Client: receive player[" << source << "] connected, state: " << packet->GetPrintInfo() << std::endl;
		break;
	}
	case Player_Disconnected:
	{
		HandlePlayerDisconnect(source);
		std::cout << "Client: receive player[" << source << "] disconnected" << std::endl;
		break;
	}
	case Full_State:
	{
		HandleUpdateState(false, source, payload);
		std::cout << "Client: player[" << source << "] update full state, state: " << payload->GetPrintInfo() << std::endl;
		break;
	}

	case Delta_State:
	{
		HandleUpdateState(true, source, payload);
		std::cout << "Client: player[" << source << "] update delta state, state: " << payload->GetPrintInfo() << std::endl;
		break;
	}
		//TODO
	default:
		std::cout << "Client: player[" << source << "] error message, error_type: " << type << std::endl;
	}
}

void NetworkedGame::HandlePlayerConnect(int id, NetworkState state) {
	auto it = serverPlayers.find(id);
	if (it != serverPlayers.end()) {
		return;
	}
	auto player = this->AddPlayerToWorld(state.position, state.orientation);
	auto nPlayer = new NetworkObject(player, id);
	serverPlayers.insert(std::pair<int, NetworkObject*>(id, nPlayer));
}

void NetworkedGame::HandlePlayerDisconnect(int id) {
	auto it = serverPlayers.find(id);
	if (it != serverPlayers.end()) {
		return;
	}
	//remove object in the world
	this->world->RemoveGameObject(it->second->GetObjectPointer(), true);
	serverPlayers.erase(it);
}

void NetworkedGame::HandleUpdateState(bool delta, int id, GamePacket* payload) {
	auto it = serverPlayers.find(id);
	if (serverPlayers.end() == it) {
		return;
	}
	it->second->ReadPacket(delta, payload);
}