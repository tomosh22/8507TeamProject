#include "MainMenuGame.h"

namespace NCL::CSC8503 {

	MainMenuGame::MainMenuGame(GameManager* manager, GameWorld* world, GameTechRenderer* renderer) : 
		GameBase(manager, world, renderer)
	{

	}

	MainMenuGame::~MainMenuGame()
	{

	}

	void MainMenuGame::Update(float dt)
	{
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE))
		{
			gameManager->CloseGame();
		}
	}

	void MainMenuGame::Render()
	{
		Debug::Print("1. Graphics Test Game", Vector2(30, 30), Debug::WHITE);
		Debug::Print("2. Single Player Game", Vector2(30, 40), Debug::WHITE);
		Debug::Print("3. Multiplayer Game", Vector2(30, 50), Debug::WHITE);

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1))
		{
			gameManager->SwitchGame(GameType::GraphicsDemo);
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2))
		{
			gameManager->SwitchGame(GameType::SinglePlayer);
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM3))
		{
			//TODO:
		}
	}

}