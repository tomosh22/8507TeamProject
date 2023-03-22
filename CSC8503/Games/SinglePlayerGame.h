#pragma once

#include "GameManager.h"

namespace NCL::CSC8503 {

	class SinglePlayerGame : public GameBase
	{
	public:
		SinglePlayerGame(GameManager* manager, GameWorld* world, GameTechRenderer* renderer);
		~SinglePlayerGame();
	};

}