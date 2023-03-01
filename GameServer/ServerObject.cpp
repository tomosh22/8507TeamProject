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
	CheckConnectExpire();
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
	auto info = ConnectInfo{ peer, tm };
	connects.insert(std::pair<int, ConnectInfo>(peer->incomingPeerID, info));
	std::cout << "Client[" << peer->incomingPeerID << "]: connected..., time: " << tm << std::endl;
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
	GamePacket* packet = DeepCopyPacket(payload);
	ENetPacket* dataPacket = enet_packet_create(packet, payload->GetTotalSize(), 0);
	if (NULL == dataPacket) {
		std::cout << "Client[" << source << "]: Create global packet error" << source << std::endl;
		return;
	}
	//todo
	//enet_host_broadcast(netHandle, 0, dataPacket);
	Broadcast(type, dataPacket, source);
	
	delete packet;
}

void ServerObject::ReplyToClient(int type, ENetPeer* peer) {
	if (!peer) { return; }
	auto msgData = new GamePacket();
	msgData->type = type;
	ENetPacket* msg = enet_packet_create(&msgData, msgData->GetTotalSize(), 0);
	if (msg) {
		enet_peer_send(peer, 0, msg);
	}
	delete msgData;
}

//broadcast
void ServerObject::Broadcast(int type, ENetPacket* msg, int source) {
	if (NULL == msg) { return; }
	for (auto it : connects) {
		int pid = it.first;
		if (source == pid) {
			continue;
		}
		enet_peer_send(it.second.GetPeer(), 0, msg);
	}
}

GamePacket* ServerObject::DeepCopyPacket(GamePacket* packet) {
	if (!packet) { return NULL; }
	switch (packet->type) {
	case Message:
		return new MessagePacket(*(MessagePacket*)packet);
	case Add_Object:
		return new AddObjectPacket(*(AddObjectPacket*)packet);
	case Full_State:
		return new FullPacket(*(FullPacket*)packet);
	case Delta_State:
		return new DeltaPacket(*(DeltaPacket*)packet);
	case Received_State:
		return new ClientPacket(*(ClientPacket*)packet);
	default:
		return new GamePacket(*packet);
	}
}