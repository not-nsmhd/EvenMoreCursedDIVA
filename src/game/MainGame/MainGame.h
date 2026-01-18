#pragma once
#include "game_state.h"
#include "gfx/Render2D/SpriteRenderer.h"

namespace DIVA::MainGame
{
	struct Context
	{
		Starshine::GFX::Render2D::SpriteRenderer* SpriteRenderer = nullptr;
		Starshine::GFX::Render2D::Font* DebugFont = nullptr;

		struct ScoreData
		{
			u32 Score{};
			u32 Combo{};
			u32 MaxCombo{};
		} Score;

		Context() {}
	};

	class MainGameState : public Starshine::GameState
	{
	public:
		MainGameState();
		~MainGameState() = default;

	public:
		bool Initialize();
		bool LoadContent();

		void UnloadContent();
		void Destroy();

		void Update(f64 deltaTime_milliseconds);
		void Draw(f64 deltaTime_milliseconds);

		std::string_view GetStateName() const;

	private:
		struct StateInternal;
		StateInternal* stateInternal{ nullptr };

		Context context;
	};
}
