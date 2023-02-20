#include "NetworkBase.h"
#include "NetworkState.h"
#include <sstream>

namespace NCL::CSC8503 {
	struct MessagePacket : public GamePacket {
		short messageID;
		std::string content;

		MessagePacket() {
			messageID = 0;
			type = Message;
			size = sizeof(MessagePacket) - sizeof(GamePacket);
		}
	};

	struct ConnectPacket :public GamePacket {
		int objectID = -1;
		NetworkState fullState;

		ConnectPacket() {
			type = Player_Connected;
			size = sizeof(ConnectPacket) - sizeof(GamePacket);
		}
		std::string GetPrintInfo() override {
			std::stringstream buffer;
			buffer << "Position: (" << fullState.position << ") Orientation: (" << fullState.orientation << ")";
			return buffer.str();
		}
	};

	struct FullPacket : public GamePacket {
		int		objectID = -1;
		NetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
		NetworkState* GetState() { return &fullState; }
		std::string GetPrintInfo() override {
			std::stringstream buffer;
			buffer << "Position: (" << fullState.position << ") Orientation: (" << fullState.orientation << ")";
			return buffer.str();
		}
	};

	struct DeltaPacket : public GamePacket {
		int		fullID = -1;
		int		objectID = -1;
		char	pos[3];
		char	orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
		std::string GetPrintInfo() override {
			std::stringstream buffer;
			buffer << "Position: (";
			for (int i = 0; i < 3; i++) {
				buffer << (float)pos[i] << ", ";
			}
			buffer << ") Orientation: (";
			for (int i = 0; i < 4; i++) {
				buffer << (float)orientation[i] << ", ";
			}
			return buffer.str();
		}
	};

	struct ClientPacket : public GamePacket {
		int		lastID;
		char	buttonstates[8];

		ClientPacket() {
			size = sizeof(ClientPacket) - sizeof(GamePacket);
		}
	};
}