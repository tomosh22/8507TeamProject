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

	struct FullPacket : public GamePacket {
		int teamId = -1;
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

	struct ActionPacket : public GamePacket {
		bool buttonstates[8] = {false, false, false, false, false, false, false, false};
		Vector3 param;
		NetworkState state;

		ActionPacket(int index, Vector3 info = Vector3(), NetworkState st = NetworkState()) {
			type = Player_Action;
			if (index < 8) {
				buttonstates[index] = true;
			}
			param = info;
			state = st;
			size = sizeof(ActionPacket) - sizeof(GamePacket);
		}
	};
}