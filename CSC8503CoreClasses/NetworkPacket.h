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

	struct AddObjectPacket :public GamePacket {
		int objectID = -1;
		NetworkState fullState;

		AddObjectPacket() {
			type = Add_Object;
			size = sizeof(AddObjectPacket) - sizeof(GamePacket);
		}
	};

	struct FullPacket : public GamePacket {
		int		objectID = -1;
		NetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
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
	};

	struct ClientPacket : public GamePacket {
		int		lastID;
		char	buttonstates[8];

		ClientPacket() {
			size = sizeof(ClientPacket) - sizeof(GamePacket);
		}
	};
}