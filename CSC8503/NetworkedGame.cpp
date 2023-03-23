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

	if (netState == CONNECTED) {
		CloseClient();
	}
}

void NetworkedGame::StartClient() {
	if (!playerObject) {
		std::cout << "There is no player in the world." << std::endl;
		return;
	}

	auto ip = NetworkBase::GetIpAddress();

	localPlayer = playerObject;
	client = new GameClient();
	if (!client->Connect(ip[0], ip[1], ip[2], ip[3], NetworkBase::GetDefaultPort())) {
		std::cout << "Client:: connect server flase, ip: " << ip[0] << "." << ip[1] << "." << ip[2] << "." << ip[3] << " port: " << NetworkBase::GetDefaultPort() << std::endl;
		return;
	}

	for (int i = BasicNetworkMessages::None; i < BasicNetworkMessages::MessageTypeMax; i++) {
		client->RegisterPacketHandler(i, this);
	}
	netState = CONNECTING;
	std::cout << "Client: connecting server, ip: " << ip[0] << "." << ip[1] << "." << ip[2] << "." << ip[3] << " port: " << NetworkBase::GetDefaultPort() << std::endl;
}

void NetworkedGame::CloseClient() {
	netState = DISCONNECT;
	if (localPlayer) {
		localPlayer->Offline();
	}
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
	if (GAME_MODE_ONLINE_GAME == gameMode) {
		if (DISCONNECT == netState) {
			StartClient();
		}
		else if (CONNECTING == netState) {
			Debug::Print("Connecting server...", Vector2(40, 30));
		}
		else if (CONNECTED == netState) {
			//Waiting for host players to choose
			//Waiting for players to connect
			//print player score
			if (!pause) {
				PrintPlayerScore();
			}
			if (playerNum > GetNetworkPlayerNum() || playerNum == 0) {
				Debug::Print("Waiting players... player num: " + std::to_string(GetNetworkPlayerNum()), Vector2(25, 25), Debug::GREEN);
				if (!pause) { pause = true; }
			}
			else {
				if (pause) {
					pause = false;
				}
			}
		}
	}
	if (client) {
		//update from server
		client->UpdateClient();
		if (CONNECTED == netState) {
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
	if (!payload) { return; }
	source = payload->networkID;
	switch (type) {
	case Connect_Confirmed:{
		HandleConnectConfirmed(payload);
		break;
	}
	case Player_Disconnected:{
		HandlePlayerDisconnect(source);
		break;
	}
	case Select_Player_Mode: {
		HandleSelectPlayerMode(payload);
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
		netState = DISCONNECT;
		break;
	}
	default:
		std::cout << "Client: player[" << source << "] error message, error_type: " << type << std::endl;
	}
}

void NetworkedGame::HandleConnectConfirmed(GamePacket* packet) {
	auto recv = (ConfirmedPacket*)packet;
 	if (localPlayer) {
		FullPacket* msg = new FullPacket(recv->networkID);
		msg->teamId = localPlayer->GetTeamId();
		msg->objectID = recv->networkID;
		msg->fullState.position = localPlayer->GetTransform().GetPosition();
		msg->fullState.orientation = localPlayer->GetTransform().GetOrientation();
		localPlayer->SetNetworkId(recv->networkID);
		localPlayer->Online();
		client->SendPacket(*msg);	//broadcast first connect;
		////Get mode
		if (localPlayer->GetNetworkId() != 0) {
			auto packet = new SelectModePacket(recv->networkID);
			client->SendPacket(*packet);
		}
	}

	std::cout << "Client: connect server confirmed! network_id: " << localPlayer->GetNetworkId() << std::endl;
	netState = CONNECTED;
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

void NetworkedGame::HandleSelectPlayerMode(GamePacket* payload) {
	auto packet = (SelectModePacket*)payload;
	if (packet) {
		playerNum = packet->mode;
	}
}

void NetworkedGame::HandleUpdateState(bool delta, int pid, GamePacket* payload) {
	if (!payload) { return; }
	auto it = serverPlayers.find(pid);
	if (serverPlayers.end() == it) {
		if (Full_State == payload->type) {
			auto fullPacket = (FullPacket*)payload;
			AddNewNetworkPlayerToWorld(fullPacket->objectID, fullPacket->teamId, fullPacket->fullState);
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

void NetworkedGame::PrintPlayerScore() {
	if (!localPlayer) { return; }
	std::string buff = "Score: ";
	vector<Vector3> colour = { Debug::RED, Debug::BLUE, Debug::GREEN, Debug::YELLOW };
	int maxIndex = localPlayer->GetTeamId();
	int maxScore = localPlayer->GetScore();
	for (int i = 0; i < 4; i++) {
		auto it = serverPlayers.find(i);
		if (serverPlayers.end() == it) {
			continue;
		}
		int score = it->second->GetScore();
		if (score > maxScore) {
			maxScore = score;
			maxIndex = i;
		}
		Debug::Print("Player [" + std::to_string(i) + "] Score: " + std::to_string(score), Vector2(60.0f, 15.0f + i * 5.0f), colour[i]);
	}
	if (gameEnded) {
		Debug::Print("Game Over, Winner: " + std::to_string(maxIndex) + ", Score: " + std::to_string(maxScore), Vector2(30, 40));
	}
}