#include "ServerObject.h"

using namespace std;

int main() {
	//init
	auto server = new ServerObject(NetworkBase::GetDefaultPort(), MAX_CLIENT_NUM);
	//server->Init();
	//while (true) {
	//	server->UpdateServer();
	//}
}
