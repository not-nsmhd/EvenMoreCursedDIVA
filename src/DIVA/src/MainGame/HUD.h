#pragma once
#include "Common/Types.h"
#include "MainGame.h"
#include "HitEvaluation.h"
#include "GFX/SpritePacker.h"

namespace DIVA::MainGame
{
	class HUD : NonCopyable
	{
	public:
		HUD(MainGameContext& context) : mainGameContext{ context } {}

		void Initialize();
		void Reset();
		bool LoadSprites(Starshine::GFX::SpritePacker& sprPacker);

		void Destroy();
		void Update(float deltaTime_ms);
		void Draw(float deltaTime_ms);

		void SetComboDisplayState(HitEvaluation hitEvaluation, u32 combo, bool wrong, vec2& position);
		void SetScoreBonusDisplayState(u32 value, vec2& position);
		void HoldScoreBonus();
		void ReleaseScoreBonus(bool drop);
	private:
		MainGameContext& mainGameContext;

		struct Impl;
		Impl* impl{ nullptr };
	};
}
