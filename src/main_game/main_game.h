#pragma once
#include "../game.h"
#include "../gfx/sprite_renderer.h"

namespace MainGame
{
	struct Context
	{
		GFX::SpriteRenderer* SpriteRenderer;

		struct ScoreData
		{
			u32 Combo{};
			u32 MaxCombo{};
		} Score;
	};

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
		StateInternal* stateInternal{ nullptr };

		Context context;
	};
}
