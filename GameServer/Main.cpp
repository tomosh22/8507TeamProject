#include "ServerObject.h"
#include <conio.h>

using namespace NCL::CSC8503;

int main() {
	//init
	NetworkBase::Initialise();
	auto server = new ServerObject(NetworkBase::GetDefaultPort(), MAX_CLIENT_NUM);
	server->Init();
	while (true) {
		while (!_kbhit()) {
			server->UpdateServer();
		}
		//escape
		if (_getch() == 27) { 
			server->Shutdown();
			break;
		}
	}
	std::cout << "Server: Shutdown..." << std::endl;
}
