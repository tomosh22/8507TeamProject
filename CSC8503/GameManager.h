#pragma once


namespace NCL {
	namespace CSC8503 {
		class GameManager
		{
		public:
			static GameManager* GetInstance()
			{
				if (_instance == nullptr)
				{
					_instance = new GameManager();
				}
				return _instance;
			}

		protected:
			static GameManager* _instance;


		};
	}
}

