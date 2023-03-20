#include "NetworkBase.h"
#include "NetworkState.h"
#include <sstream>

namespace NCL::CSC8503 {
	struct MessagePacket : public GamePacket {
		short messageID;
		std::string content;

		MessagePacket(int nid) {
			networkID = nid;
			messageID = 0;
			type = Message;
			size = sizeof(MessagePacket) - sizeof(GamePacket);
		}
	};

	struct ConfirmedPacket : public GamePacket {
		ConfirmedPacket(int nid = 0) {
			networkID = nid;
			type = Connect_Confirmed;
			size = sizeof(ConfirmedPacket) - sizeof(GamePacket);
		}
	};

	struct SelectModePacket : public GamePacket {
		short mode;
		SelectModePacket(int nid, int mode = 0) : mode(mode) {
			networkID = nid;
			type = Select_Player_Mode;
			size = sizeof(SelectModePacket) - sizeof(GamePacket);
		}
	};

	struct FullPacket : public GamePacket {
		int teamId = -1;
		int		objectID = -1;
		NetworkState fullState;

		FullPacket(int nid) {
			networkID = nid;
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public GamePacket { 
		int		fullID = -1;
		int		objectID = -1;
		char	pos[3];
		char	orientation[4];

		DeltaPacket(int nid) {
			networkID = nid;
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	struct ActionPacket : public GamePacket {
		bool buttonstates[8] = {false, false, false, false, false, false, false, false};
		Vector3 param;
		NetworkState state;

		ActionPacket(int nid, int index, Vector3 info = Vector3(), NetworkState st = NetworkState()) {
			type = Player_Action;
			if (index < 8) {
				buttonstates[index] = true;
			}
			networkID = nid;
			param = info;
			state = st;
			size = sizeof(ActionPacket) - sizeof(GamePacket);
		}
	};
}