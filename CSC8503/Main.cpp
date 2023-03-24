#include "Window.h"

#include "Games/GameManager.h"

using namespace NCL;
using namespace CSC8503;

#if 0
NetworkedGame::GetInstance()->UpdateGame(dt);
#endif

int main() {
	Window*w = Window::CreateGameWindow("CSC8508 Game Demo", 1280, 720);
	
	if (!w->HasInitialised()) {
		return -1;
	}	

	w->SeizeMouse(true);

	GameManager* gameManager = new GameManager(w);

	while (w->UpdateWindow() && !gameManager->ShouldExit()) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();

		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}

		if (w->GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}

		if (w->GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		if (w->GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}

		gameManager->Update(dt);
	}

	delete gameManager;

	Window::DestroyGameWindow();
}