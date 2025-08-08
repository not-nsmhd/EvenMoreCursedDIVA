#pragma once
#include "../game.h"

namespace MainGame
{
	class MainGameState : public GameState
	{
	public:
		MainGameState();
		~MainGameState();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();

	private:
		struct StateInternal;
		StateInternal* stateInternal = nullptr;
	};
}
