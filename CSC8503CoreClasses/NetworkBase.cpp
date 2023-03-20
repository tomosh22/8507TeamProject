#include <sstream>
#include <iostream>
#include <fstream>

#include "NetworkBase.h"
#include "./enet/enet.h"
#include "../NCLCoreClasses/Assets.h"
NetworkBase::NetworkBase() {
	netHandle = nullptr;
}

NetworkBase::~NetworkBase() {
	if (netHandle) {
		enet_host_destroy(netHandle);
	}
}

void NetworkBase::Initialise() {
	auto split_string = [](std::string str, char divide, std::vector<int> ret) {
		std::stringstream ss(str);
		std::string b;
		while (std::getline(ss, b, divide)) {
			float tmp = std::stoi(b);
			ret.push_back(tmp);
		}
	};

	//load config
	std::string fileData;
	NCL::Assets::ReadTextFile(NCL::Assets::CONFIGDIR + NETWORK_FILE, fileData);
	std::stringstream data(fileData);
	std::string buff;
	while (std::getline(data, buff)) {
		size_t pos = buff.find(':');
		if (std::string::npos == pos) { continue; }
		std::string tp = buff.substr(0, pos);
		std::string info = buff.substr(pos + 1);
		if ("ip" == tp) {
			std::vector<int> ip;
			std::stringstream ss(info);
			std::string b;
			while (std::getline(ss, b, '.')) {
				float tmp = std::stoi(b);
				ip.push_back(std::stoi(b));
			}
			glIpAdress = ip;
			continue;
		}
		if ("port" == tp) {
			glDefaultPort = std::stoi(info);
		}
	}
	//init network
	enet_initialize();
}

void NetworkBase::Destroy() {
	enet_deinitialize();
}

bool NetworkBase::ProcessPacket(GamePacket* packet, int peerID) {
	PacketHandlerIterator firstHandler;
	PacketHandlerIterator lastHandler;

	bool canHandle = GetPacketHandlers(packet->type, firstHandler, lastHandler);
	if (canHandle) {
		for (auto i = firstHandler; i != lastHandler; ++i) {
			i->second->ReceivePacket(packet->type, packet, peerID);
		}
		return true;
	}
	std::cout << __FUNCTION__ << "no handler for packet type: " << packet->type << std::endl;
	return false;
}