#pragma once
#include "Common/Types.h"
#include "MainGame.h"
#include "HitEvaluation.h"

namespace DIVA::MainGame
{
	class HUD : NonCopyable
	{
	public:
		HUD(MainGameContext& context);
		~HUD();

		void Initialize();
		void Reset();
		bool LoadSprites(Starshine::Graphics::SpritePacker& sprPacker);

		void Destroy();
		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		void SetComboDisplayState(HitEvaluation hitEvaluation, u32 combo, bool wrong, vec2& position);
		void SetScoreBonusDisplayState(u32 value, vec2& position);
		void HoldScoreBonus();
		void ReleaseScoreBonus(bool drop);

		void SetLyricsText(std::string_view text, const Starshine::Color& color = Starshine::DefaultColors::White);
	private:
		MainGameContext& mainGameContext;

		struct Impl;
		Impl* impl{};
	};
}
