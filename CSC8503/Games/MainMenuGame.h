#pragma once

#include "GameManager.h"

namespace NCL::CSC8503 {

	class MainMenuGame : public GameBase
	{
	public:
		MainMenuGame(GameManager* manager, GameWorld* world, GameTechRenderer* renderer);
		~MainMenuGame();

		void Update(float dt) override;
		void Render() override;
	};

}