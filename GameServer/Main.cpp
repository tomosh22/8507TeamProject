#include "ServerObject.h"
#include <conio.h>

using namespace std;

int main() {
	//init
	NetworkBase::Initialise();
	auto server = new NCL::CSC8503::ServerObject(NetworkBase::GetDefaultPort(), 4);
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
