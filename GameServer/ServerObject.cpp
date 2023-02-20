#include "ServerObject.h"

void ServerObject::Init() {
	NetworkBase::Initialise();
	Initialise();
	for (int i = BasicNetworkMessages::None; i < BasicNetworkMessages::MessageTypeMax; i++) {
		RegisterPacketHandler(i, this);
	}
}

void ServerObject::UpdateServer() {
	if (!netHandle) { return; }
	ENetEvent event;
	//auto tm = std::time(0);
	while (enet_host_service(netHandle, &event, 0) > 0) {
		auto tm = 0;
		//tm = std::time(0);
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;
		GamePacket* packet = (GamePacket*)event.packet->data;
		if (ENetEventType::ENET_EVENT_TYPE_CONNECT == type) {
			auto info = ConnectInfo{ event.peer, (float)tm };
			connects.insert(std::pair<int, ConnectInfo>(peer, info));
			std::cout << "Client[" << peer << "]: connected..., time: " << tm << std::endl;
			ProcessPacket(packet, peer);
		}
		else if (ENetEventType::ENET_EVENT_TYPE_DISCONNECT == type) {
			connects.erase(peer);
			std::cout << "Client[" << peer << "]: has disconnected..." << std::endl;
			ProcessPacket(packet, peer);
		}
		else if (ENetEventType::ENET_EVENT_TYPE_RECEIVE == type) {
			auto source = connects.find(peer);
			if (connects.end() != source) {
				source->second.UpdateTime(tm);
				ProcessPacket(packet, peer);
			}
		}
		else {
			std::cout << "Client[" << peer << "]: error message" << std::endl;
		}
		enet_packet_destroy(event.packet);
	}
	CheckConnects();
}

//check if exception has occurred on clients 
void ServerObject::CheckConnects() {
	//float tm = std::time(0);
	float tm = 0;
	for (auto it : connects) {
		if (it.second.Expire(tm)) {
			connects.erase(it.first);
		}
	}
}

//handle packet
//@param type - message type
//@param payload - message
//@param source - source peer
void ServerObject::ReceivePacket(int type, GamePacket* payload, int source) {
	ENetPacket* dataPacket = enet_packet_create(&payload, payload->GetTotalSize(), 0);
	if (NULL == dataPacket) {
		std::cout << "Client[" << source << "]: Create global packet error" << source << std::endl;
		return;
	}
	//broadcast
	for (auto it : connects) {
		int pid = it.first;
		if (source == pid) {
			continue;
		}
		std::cout << "Client[" << source << "]: Send to [" << pid << "], message_type: " << payload->GetType() << std::endl;
		enet_peer_send(it.second.GetPeer(), 0, dataPacket);
	}
}