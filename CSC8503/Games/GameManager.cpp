#include "GameManager.h"

#include "GraphicsGame.h"
#include "MainMenuGame.h"
#include "SinglePlayerGame.h"

#include <psapi.h>

namespace NCL::CSC8503 {

	GameManager::GameManager(Window* w) : 
		window(w)
	{
		renderer = new GameTechRenderer(window);

		ActivateGameBase(GameType::MainMenu);
	}

	GameManager::~GameManager()
	{
		DeactivateGameBase();

		delete renderer;
	}

	void GameManager::Update(float dt)
	{
		//Switch game, if needed
		if (shouldSwitchGameType)
		{
			shouldSwitchGameType = false;

			DeactivateGameBase();
			ActivateGameBase(desiredGameType);
		}

		//Begin a new ImGui frame. We want to begin at the very start so that everything
		//else is free to use ImGui whenever they want to
		renderer->timePassed += dt;
		renderer->BeginImGui();
		
		//Update and render the game
		activeGame->Update(dt);
		activeGame->Render();

		//Update the world
		activeWorld->UpdateWorld(dt);
		renderer->SetActiveCamera(activeWorld->GetMainCamera());

		//Submit all render objects to the renderer
		activeWorld->OperateOnContents([&](GameObject* o)
		{
			if (!o->IsActive()) return;

			if (o->GetRenderObject())
				renderer->SubmitRenderObject(o->GetRenderObject());
		});

		//Draw the debug overlay
		if (debugOverlay)
		{
			DrawImGuiGameStats();
		}

		//End the current ImGui frame and render
		renderer->EndImGui();

		renderer->Update(dt);
		renderer->Render();

		Debug::UpdateRenderables(dt);
	}

	void GameManager::SwitchGame(GameType game)
	{
		shouldSwitchGameType = true;
		desiredGameType = game;
	}

	void GameManager::CloseGame()
	{
		shouldCloseGame = true;
	}

	bool GameManager::ShouldExit() const
	{
		return shouldCloseGame;
	}

	void GameManager::DrawImGuiGameStats()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

		float padding = 10.0f;
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;

		ImVec2 window_pos(work_pos.x + work_size.x - padding, work_pos.y + work_size.y - padding);
		ImVec2 window_pos_pivot(1.0f, 1.0f);

		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowBgAlpha(0.35f);

		bool alwaysTrue = true;

		if (ImGui::Begin("Stats Overlay", &alwaysTrue, window_flags))
		{
			ImGui::Text("Game Stats");
			ImGui::Separator();

			//Display the last frame time
			float lastFrameTime = window->GetTimer()->GetTimeDeltaSeconds();
			int lastFrameRate = (int)(1.0f / lastFrameTime);

			ImGui::Text("Frame rate: %d (%.2f ms)", lastFrameRate, 1000.0f * lastFrameTime);

			//Display the memory usage of the current process
			{
				PROCESS_MEMORY_COUNTERS memoryCounters = {};
				if (!GetProcessMemoryInfo(GetCurrentProcess(), &memoryCounters, sizeof(PROCESS_MEMORY_COUNTERS))) memoryCounters = {};

				SIZE_T memoryUsage = memoryCounters.WorkingSetSize;

				const uint64_t kilobytes = 1024;
				const uint64_t megabytes = 1024 * kilobytes;
				const uint64_t gigabytes = 1024 * megabytes;

				const char* prompt = "Memory usage: ";

				if (memoryUsage >= gigabytes) ImGui::Text("%s%.2f GB", prompt, (double)memoryUsage / gigabytes);
				else if (memoryUsage >= megabytes) ImGui::Text("%s%.2f MB", prompt, (double)memoryUsage / megabytes);
				else if (memoryUsage >= kilobytes) ImGui::Text("%s%.2f KB", prompt, (double)memoryUsage / kilobytes);
				else ImGui::Text("%s%d B", prompt, (int)memoryUsage);
			}
		}

		ImGui::End();
	}

	void GameManager::ActivateGameBase(GameType game)
	{
		activeWorld = new GameWorld();

		switch (game)
		{
		case GameType::MainMenu:
			activeGame = new MainMenuGame(this, activeWorld, renderer);
			break;
		case GameType::GraphicsDemo:
			activeGame = new GraphicsGame(this, activeWorld, renderer);
			break;
		case GameType::SinglePlayer:
			activeGame = new SinglePlayerGame(this, activeWorld, renderer);
			break;
		default:
			activeGame = nullptr;
		}
	}

	void GameManager::DeactivateGameBase()
	{
		delete activeGame;
		delete activeWorld;
	}

}
