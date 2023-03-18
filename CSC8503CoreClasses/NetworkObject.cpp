#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(int id) : GameObject(){
	deltaErrors = 0;
	fullErrors = 0;
	networkID = id;
	lastFullState = NetworkState();
}

NetworkObject::~NetworkObject()	{
}

bool NetworkObject::ReadPacket(bool delta, GamePacket& p) {
	if (delta) {
		DeltaPacket packet = (DeltaPacket&)p;
		ReadDeltaPacket(packet);
		return true;
	}
	FullPacket packet = (FullPacket&)p;
	ReadFullPacket(packet.fullState);
	return false;
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
	}
	return WriteFullPacket(p);
}
//Client objects receive these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) {
	if (p.fullID != lastFullState.stateID) {
		return false;
	}
	UpdateStateHistory(p.fullID);

	Vector3 fullPos = lastFullState.position;
	Quaternion fullOrientation = lastFullState.orientation;

	fullPos.x += p.pos[0];
	fullPos.y += p.pos[1];
	fullPos.z += p.pos[2];

	fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
	fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
	fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
	fullOrientation.y += ((float)p.orientation[3]) / 127.0f;

	transform.SetPosition(fullPos);
	transform.SetOrientation(fullOrientation);
	return true;
}

bool NetworkObject::ReadFullPacket(NetworkState &state) {
	if (state.stateID < lastFullState.stateID) {
		return false;
	}
	lastFullState = state;

	transform.SetPosition(lastFullState.position);
	transform.SetOrientation(lastFullState.orientation);

	stateHistory.emplace_back(lastFullState);

	return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket**p, int stateID) {
	DeltaPacket* dp = new DeltaPacket();
	NetworkState state;
	if (!GetNetworkState(stateID, state)) {
		return false;
	}

	dp->fullID = stateID;
	dp->objectID = networkID;

	Vector3 currentPos = transform.GetPosition();
	Quaternion currentOrientation = transform.GetOrientation();

	currentPos -= state.position;
	currentOrientation -= state.orientation;

	dp->pos[0] = (char)currentPos.x;
	dp->pos[1] = (char)currentPos.y;
	dp->pos[2] = (char)currentPos.z;

	dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
	dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
	dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
	dp->orientation[3] = (char)(currentOrientation.w * 127.0f);
	*p = dp;
	return true;
}

bool NetworkObject::WriteFullPacket(GamePacket**p) {
	FullPacket* fp = new FullPacket();

	fp->objectID = networkID;
	fp->fullState.position = transform.GetPosition();
	fp->fullState.orientation = transform.GetOrientation();
	fp->fullState.stateID = lastFullState.stateID++;
	*p = fp;
	return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}

bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
	for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
		if ((*i).stateID == stateID) {
			state = (*i);
			return true;
		}
	}
	return false;
}

void NetworkObject::UpdateStateHistory(int minID) {
	for (auto i = stateHistory.begin(); i < stateHistory.end();) {
		if ((*i).stateID < minID) {
			i = stateHistory.erase(i);
		}
		else {
			++i;
		}
	}
}