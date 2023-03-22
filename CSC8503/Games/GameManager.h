#pragma once

#include "Window.h"
#include "GameTechRenderer.h"
#include "GameWorld.h"

namespace NCL::CSC8503 {

	enum class GameType
	{
		MainMenu,
		GraphicsDemo,
		SinglePlayer,
	};

	class GameManager;

	class GameBase
	{
	protected:
		GameManager* gameManager = nullptr;
		GameWorld* world = nullptr;
		GameTechRenderer* renderer = nullptr;
	public:
		GameBase(GameManager* manager, GameWorld* world, GameTechRenderer* renderer) :
			gameManager(manager), world(world), renderer(renderer) {}
		virtual ~GameBase() {}

		virtual void Update(float dt) {}
		virtual void Render() {}
	};

	class GameManager
	{
	private:
		Window* window = nullptr;
		GameTechRenderer* renderer = nullptr;

		GameWorld* activeWorld = nullptr;
		GameBase* activeGame = nullptr;

		GameType desiredGameType = GameType::MainMenu;
		bool shouldSwitchGameType = false;
		bool shouldCloseGame = false;

		bool debugOverlay = true;
	private:
		void DrawImGuiGameStats();

		void ActivateGameBase(GameType game);
		void DeactivateGameBase();
	public:
		GameManager(Window* window);
		~GameManager();

		void Update(float dt);
		void SwitchGame(GameType game);
		void CloseGame();

		bool ShouldExit() const;
	};

}