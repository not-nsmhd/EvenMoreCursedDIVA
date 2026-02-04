#pragma once
#include "Common/Types.h"
#include "Chart.h"
#include <GameInstance.h>
#include <Rendering/Render2D/SpriteRenderer.h>

namespace DIVA::MainGame
{
	struct MainGameContext
	{
		Starshine::Rendering::Render2D::SpriteRenderer* SpriteRenderer{};
		Starshine::Rendering::Render2D::Font* DebugFont{};

		struct ScoreData
		{
			u32 Score{};
			u32 Combo{};
			u32 MaxCombo{};
		} Score;

		struct IconSetSpritesCache
		{
			Starshine::Rendering::Render2D::SpriteSheet SpriteSheet{};

			const Starshine::Rendering::Render2D::Sprite* NoteTargets[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Rendering::Render2D::Sprite* NoteIcons[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Rendering::Render2D::Sprite* NoteTargetHand{};

			const Starshine::Rendering::Render2D::Sprite* DoubleNoteTargets[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Rendering::Render2D::Sprite* DoubleNoteIcons[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Rendering::Render2D::Sprite* DoubleNoteTargetHands[Starshine::EnumCount<NoteShape>()]{};

			const Starshine::Rendering::Render2D::Sprite* HoldNoteTargets[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Rendering::Render2D::Sprite* HoldNoteIcons[Starshine::EnumCount<NoteShape>()]{};

			const Starshine::Rendering::Render2D::Sprite* HoldNoteTrails[Starshine::EnumCount<NoteShape>()]{};

			const Starshine::Rendering::Render2D::Sprite* Trail_Normal{};
			const Starshine::Rendering::Render2D::Sprite* Trail_CT{};
		} IconSetSprites;

		MainGameContext() {}
	};

	class MainGameState : public Starshine::GameState
	{
	public:
		MainGameState();
		~MainGameState();

	public:
		bool Initialize();
		bool LoadContent();

		void UnloadContent();
		void Destroy();

		void Update(f64 deltaTime_milliseconds);
		void Draw(f64 deltaTime_milliseconds);

		std::string_view GetStateName() const;

	public:
		struct LoadSetttingsData
		{
			std::string ChartPath;
		} LoadSettings;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};

		MainGameContext context;
	};
}
