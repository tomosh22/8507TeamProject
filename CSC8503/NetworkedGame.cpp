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
	if (!playerObject) {
		std::cout << "There is no player in the world." << std::endl;
		return;
	}

	localPlayer = playerObject;
	client = new GameClient();
	if (!client->Connect(a, b, c, d, NetworkBase::GetDefaultPort())) {
		std::cout << "Client:: connect server flase, ip: " << a << "." << b << "." << c << "." << d << " port: " << NetworkBase::GetDefaultPort() << std::endl;
		return;
	}

	for (int i = BasicNetworkMessages::None; i < BasicNetworkMessages::MessageTypeMax; i++) {
		client->RegisterPacketHandler(i, this);
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
	if (online) {
		Debug::Print("Network: on, Press F10 to exit network.", Vector2(1, 5));
	}
	else {
		Debug::Print("Network: off, Press F9 to start network.", Vector2(1, 5));
	}
	
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
			UpdateToServer(dt);
		}
	}

	TutorialGame::UpdateGame(dt);

}

void NetworkedGame::UpdateToServer(float dt) {
	//broadcast
	BroadcastSnapshot();
}

void NetworkedGame::BroadcastSnapshot() {
	if (!localPlayer) { return; }
	int playerState = 0;
	GamePacket* newPacket = nullptr;
	if (localPlayer->WritePacket(&newPacket, Full_State, playerState)) {
		client->SendPacket(*newPacket);
	}
	//send player's action.
	auto messages = localPlayer->GetSendMessages();
	if (messages.size() != 0) {
		for (auto it : messages) {
			client->SendPacket(*it);
		}
		localPlayer->ClearMessagePool();
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
		break;
	}
	case Player_Disconnected:{
		HandlePlayerDisconnect(source);
		break;
	}
	case Full_State:{
		HandleUpdateState(false, source, payload);
		break;
	}
	case Delta_State:{
		HandleUpdateState(true, source, payload);
		break;
	}
	case Player_Action: {
		HandlePlayerAction(source, payload);
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
	FullPacket* msg = new FullPacket();
	if (localPlayer) {
		msg->teamId = localPlayer->GetTeamId();
		msg->fullState.position = localPlayer->GetTransform().GetPosition();
		msg->fullState.orientation = localPlayer->GetTransform().GetOrientation();
	}
	client->SendPacket(*msg);	//broadcast first connect;
	std::cout << "Client: connect server confirmed!" << std::endl;
	online = true;
}

void NetworkedGame::HandlePlayerDisconnect(int pid) {
	std::cout << "Client: receive player[" << pid << "] disconnected" << std::endl;
	auto it = serverPlayers.find(pid);
	if (it != serverPlayers.end()) {
		return;
	}
	//remove all objects of this server in the world
	GameWorld::GetInstance()->RemoveGameObject(it->second, true);
	serverPlayers.erase(it);
}

void NetworkedGame::HandleUpdateState(bool delta, int pid, GamePacket* payload) {
	if (!payload) { return; }
	auto it = serverPlayers.find(pid);
	if (serverPlayers.end() == it) {
		if (Full_State == payload->type) {
			auto fullPacket = (FullPacket*)payload;
			AddNewNetworkPlayerToWorld(pid, fullPacket->teamId, fullPacket->fullState);
		}
		return;
	}
	it->second->ReadPacket(delta, *payload);
}

void NetworkedGame::AddNewNetworkPlayerToWorld(int pid, int teamID, NetworkState state) {
	auto it = serverPlayers.find(pid);
	if (it != serverPlayers.end()) { return; } //player already exist;
	auto player = this->AddPlayerToWorld(state.position, state.orientation);
	//todo
	player->SetTeamId(teamID);
	player->SetNetworkId(pid);
	serverPlayers[pid] = player;
	std::cout << "Client: receive new player[" << pid << "] connected, position: " << state.position << "orientation: " << state.orientation << "teamID:" << teamID << std::endl;
}

void NetworkedGame::HandlePlayerAction(int pid, GamePacket* payload) {
	if (!payload) { return; }
	auto it = serverPlayers.find(pid);
	if (it == serverPlayers.end()) { return; }
	ActionPacket* packet = (ActionPacket*)payload;
	it->second->UpdateAction(*packet);
}