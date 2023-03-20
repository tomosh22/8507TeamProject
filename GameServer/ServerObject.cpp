#include "ServerObject.h"

using namespace NCL;
using namespace CSC8503;

void ServerObject::Init() {
	for (int i = BasicNetworkMessages::None; i < BasicNetworkMessages::MessageTypeMax; i++) {
		RegisterPacketHandler(i, this);
	}
	std::cout << "Server: Initialization success" << std::endl;
}

void ServerObject::UpdateServer() {
	if (!netHandle) { return; }
	ENetEvent event;
	auto tm = std::time(0);
	while (enet_host_service(netHandle, &event, 0) > 0) {
		
		int type = event.type;
		ENetPeer* peer = event.peer;
		int pid = peer->incomingPeerID;
		if (ENetEventType::ENET_EVENT_TYPE_CONNECT == type) {
			HandleConnected(peer);
		}
		else if (ENetEventType::ENET_EVENT_TYPE_DISCONNECT == type) {

			HandleDisconnected(pid);
		}
		else if (ENetEventType::ENET_EVENT_TYPE_RECEIVE == type) {
			HandleReceived(pid, (GamePacket*)event.packet->data);
		}
		else {
			std::cout << "Client[" << pid << "]: error message" << std::endl;
		}
		enet_packet_destroy(event.packet);
	}
	//CheckConnectExpire();
}

//check if exception has occurred on clients 
void ServerObject::CheckConnectExpire() {
	auto tm = time(0);
	for (auto it : connects) {
		if (it.second.Expire(tm)) {
			std::cout << "Expire" << it.second.updateTime + CONNECT_CHECK_OFFSET << ":" << tm << std::endl;
			HandleDisconnected(it.first);
			break;
		}
	}
}

void ServerObject::HandleConnected(ENetPeer* peer) {
	if (!peer) { return; }
	auto tm = time(0);
	auto info = ConnectInfo{0, tm, peer };
	connects.insert(std::pair<int, ConnectInfo>(peer->incomingPeerID, info));
	std::cout << "Client[" << peer->incomingPeerID << "]: connected..., time: " << tm << std::endl;
	auto sp = new ConfirmedPacket(peer->incomingPeerID);
	ReplyToClient(peer->incomingPeerID, sp);
}

void ServerObject::HandleDisconnected(int pid) {
	connects.erase(pid);
	auto packet = new GamePacket();
	packet->type = Player_Disconnected;
	ProcessPacket(packet, pid);
	delete packet;
	std::cout << "Client[" << pid << "]: has disconnected..." << std::endl;
}

void ServerObject::HandleReceived(int pid, GamePacket* packet) {
	if (!packet) { return; }
	auto it = connects.find(pid);
	if (connects.end() == it) {
		return;
	}
	auto tm = time(0);
	it->second.UpdateTime(tm);
	ProcessPacket(packet, pid);
}

//handle packet
//@param type - message type
//@param payload - message
//@param source - source peer
void ServerObject::ReceivePacket(int type, GamePacket* payload, int source) {
	if (!payload) { return; }

	//Host player selection mode
	if (Select_Player_Mode == type) {
		if (HOST_PLAYER_PID == source) {
			auto it = connects.find(source);
			if (connects.end() == it) { return; }
			SelectModePacket* sp = (SelectModePacket*)payload;
			it->second.gameMode = sp->mode;
			std::cout << "Save mode: " << it->second.gameMode << std::endl;
		} else {
			auto it = connects.find(HOST_PLAYER_PID);
			if (connects.end() == it) { return; }
			auto sp = new SelectModePacket(source, it->second.gameMode);
			ReplyToClient(source, sp);
			return;
		}
	}

	//todo
	//enet_host_broadcast(netHandle, 0, dataPacket);
	Broadcast(type, payload, source);
	
}

void ServerObject::ReplyToClient(int pid, GamePacket* packet) {
	auto it = connects.find(pid);
	if (connects.end() == it) { return; }
	auto peer = it->second.GetPeer();
	ENetPacket* msg = enet_packet_create(packet, packet->GetTotalSize(), 0);
	if (msg) {
		enet_peer_send(peer, 0, msg);
	}
}

//broadcast
void ServerObject::Broadcast(int type, GamePacket* payload, int source) {
	if (!payload) { return; }

	for (auto it : connects) {
		GamePacket* packet = DeepCopyPacket(payload);
		ENetPacket* msg = enet_packet_create(packet, packet->GetTotalSize(), 0);
		if (!msg) { 
			return;
		}

		int pid = it.first;
		if (source == pid) {
			continue;
		}
		enet_peer_send(it.second.GetPeer(), 0, msg);
		delete packet;
	}
}

GamePacket* ServerObject::DeepCopyPacket(GamePacket* packet) {
	if (!packet) { return NULL; }
	switch (packet->type) {
	case Message:
		return new MessagePacket(*(MessagePacket*)packet);
	case Full_State:
		return new FullPacket(*(FullPacket*)packet);
	case Delta_State:
		return new DeltaPacket(*(DeltaPacket*)packet);
	case Player_Action:
		return new ActionPacket(*(ActionPacket*)packet);
	case Select_Player_Mode:
		return new SelectModePacket(*(SelectModePacket*)packet);
	default:
		return new GamePacket(*packet);
	}
}