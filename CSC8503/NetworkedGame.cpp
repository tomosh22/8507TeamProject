#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"

#define COLLISION_MSG 30
#define NETWORK_ID_OFFSET 1000

NetworkedGame* NetworkedGame::_instance = nullptr;
NetworkedGame::NetworkedGame() {
	client = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket = 0.0f;
	packetsToSnapshot = 0;
}

NetworkedGame::~NetworkedGame() {
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

	for (int i = BasicNetworkMessages::None; i < BasicNetworkMessages::MessageTypeMax; i++) {
		client->RegisterPacketHandler(i, this);
	}
	if (selectionObject) {
		localPlayer = new NetworkObject(selectionObject, 0);
	}
	std::cout << "Client: connecting server" << std::endl;
}

void NetworkedGame::CloseClient() {
	online = false;
	if (client) {
		client->Destroy();
	}
	for (auto i : serverPlayers) {
		if (i.second) {
			delete i.second;
		}
		serverPlayers.erase(i.first);
	}
	std::cout << "Client: connect server closed..." << std::endl;
}

void NetworkedGame::UpdateGame(float dt) {
	if (!client && Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) { StartClient(127, 0, 0, 1); }
	if (client && Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) { CloseClient(); }
	if (client) {
		//update from server
		client->UpdateClient();
		if (online) {
			//broadcast
			timeToNextPacket -= dt;
			if (timeToNextPacket < 0) {
				UpdateToServer(dt);
				timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
			}
		}
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
	//broadcast
	BroadcastSnapshot(false);
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	if (!localPlayer) { return; }
	int playerState = 0;
	GamePacket* newPacket = nullptr;
	if (localPlayer->WritePacket(&newPacket, deltaFrame, playerState)) {
		client->SendPacket(*newPacket);
		delete newPacket;
	}
}

void NetworkedGame::UpdateMinimumState() {
	int minID = INT_MAX;
	int maxID = 0; //if a player is lagging behind

	for (auto i : stateIDs) {
		minID = min(minID, i.second);
		maxID = max(maxID, i.second);
	}

	for (auto it : serverPlayers) {
		if (!it.second) {
			continue;
		}
		it.second->UpdateStateHistory(minID); //clear out old states so they aren't taking up memory...
	}
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	switch (type) {
	case Connect_Confirmed:{
		HandleConnectConfirmed();
		break;}
	case Add_Object:{
		HandlePlayerConnect(source, *(AddObjectPacket*)payload);
		break;}
	case Player_Disconnected:{
		HandlePlayerDisconnect(source);
		break;}
	case Full_State:{
		HandleUpdateState(false, source, payload);
		break;}
	case Delta_State:{
		HandleUpdateState(true, source, payload);
		break;
	}
	case Shutdown: {
		std::cout << "Client: server disconnected!" << std::endl;
		online = false;
		break;
	}
	default:
		std::cout << "Client: player[" << source << "] error message, error_type: " << type << std::endl;
	}
}

void NetworkedGame::HandleConnectConfirmed() {
	AddObjectPacket* msg = new AddObjectPacket();
	if (localPlayer) {
		msg->fullState.position = localPlayer->GetObjectPointer()->GetTransform().GetPosition();
		msg->fullState.orientation = localPlayer->GetObjectPointer()->GetTransform().GetOrientation();
	}
	client->SendPacket(*msg);	//broadcast first connect;
	std::cout << "Client: connect server confirmed!" << std::endl;
	online = true;
}

void NetworkedGame::HandlePlayerConnect(int pid, AddObjectPacket& packet) {
	AddNewNetworkPlayerToWorld(pid, packet.fullState);
}

void NetworkedGame::HandlePlayerDisconnect(int pid) {
	std::cout << "Client: receive player[" << pid << "] disconnected" << std::endl;
	auto it = serverPlayers.find(pid);
	if (it != serverPlayers.end()) {
		return;
	}
	//remove all objects of this server in the world
	GameWorld::GetInstance()->RemoveGameObject(it->second->GetObjectPointer(), true);
	serverPlayers.erase(it);
}

void NetworkedGame::HandleUpdateState(bool delta, int pid, GamePacket* payload) {
	if (!payload) { return; }
	auto it = serverPlayers.find(pid);
	if (serverPlayers.end() == it) {
		if (Full_State == payload->type) {
			auto fullPacket = (FullPacket*)payload;
			AddNewNetworkPlayerToWorld(pid, fullPacket->fullState);
		}
		return;
	}
	it->second->ReadPacket(delta, payload);

}

void NetworkedGame::AddNewNetworkPlayerToWorld(int pid, NetworkState state) {
	auto it = serverPlayers.find(pid);
	if (it != serverPlayers.end()) { return; } //player already exist;
	auto player = this->AddPlayerToWorld(state.position, state.orientation);
	auto nPlayer = new NetworkObject(player, pid);
	serverPlayers.insert(std::pair<int, NetworkObject*>(pid, nPlayer));
	std::cout << "Client: receive new player[" << pid << "] connected, position: " << state.position << "orientation: " << state.orientation << std::endl;
}