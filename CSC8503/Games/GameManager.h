#pragma once

#include "Window.h"
#include "GameTechRenderer.h"
#include "GameWorld.h"

#include <mutex>

namespace NCL::CSC8503 {
	class PaintSphere : public GameObject
	{
	public:
		Vector3 center;
		float radius;
		Vector3 color;
	};
	struct TextureThing {
		char* texData = nullptr;
		int width = 0;
		int height = 0;
		int channels = 0;
		int flags = 0;

		TextureBase** myPointer;

		TextureThing(char* texData, int width, int height, int channels, int flags, TextureBase** myPointer) : texData(texData), width(width), height(height), channels(channels), flags(flags), myPointer(myPointer) {}
	};

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