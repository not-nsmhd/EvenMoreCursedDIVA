#pragma once
#include "Common/Types.h"
#include "Chart.h"
#include <GameInstance.h>
#include <Rendering/Render2D/SpriteRenderer.h>
#include <Graphics/AnimationSet.h>

namespace DIVA::MainGame
{
	struct MainGameContext
	{
		Starshine::Rendering::Render2D::SpriteRenderer* SpriteRenderer{};
		Starshine::Graphics::Font* DebugFont{};

		struct ScoreData
		{
			u32 Score{};
			u32 Combo{};
			u32 MaxCombo{};
		} Score;

		struct IconSetSpritesCache
		{
			std::shared_ptr<Starshine::Graphics::SpriteSheet> SpriteSheet{};

			const Starshine::Graphics::Sprite* NoteTargets[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Graphics::Sprite* NoteIcons[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Graphics::Sprite* NoteTargetHand{};

			const Starshine::Graphics::Sprite* DoubleNoteTargets[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Graphics::Sprite* DoubleNoteIcons[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Graphics::Sprite* DoubleNoteTargetHands[Starshine::EnumCount<NoteShape>()]{};

			const Starshine::Graphics::Sprite* HoldNoteTargets[Starshine::EnumCount<NoteShape>()]{};
			const Starshine::Graphics::Sprite* HoldNoteIcons[Starshine::EnumCount<NoteShape>()]{};

			const Starshine::Graphics::Sprite* HoldNoteTrails[Starshine::EnumCount<NoteShape>()]{};

			const Starshine::Graphics::Sprite* Trail_Normal{};
			const Starshine::Graphics::Sprite* Trail_CT{};
		} IconSetSprites;

		struct IconSetAnimationCache
		{
			std::unique_ptr<Starshine::Graphics::AnimationSet> Animations{};

			const Starshine::Graphics::Layer* NoteAppearLayer{};
			const Starshine::Graphics::Layer* NoteDisappearLayer{};

			const Starshine::Graphics::Animation* NoteAppearEffect{};
		} IconSetAnimations;

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

		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		inline static std::string_view GetStateName() { return "MainGameState"; };

	public:
		struct LoadSetttingsData
		{
			std::string ChartPath;
			std::string LyricsPath;
			std::string MusicPath;
		} LoadSettings;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};

		MainGameContext context;
	};
}
