#pragma once
#include "common/types.h"
#include "MainGame.h"
#include "HitEvaluation.h"
#include "GFX/SpritePacker.h"

namespace DIVA::MainGame
{
	class HUD : NonCopyable
	{
	public:
		HUD(Context& context) : mainGameContext{ context } {}

		void Initialize();
		void Reset();
		bool LoadSprites(Starshine::GFX::SpritePacker& sprPacker);

		void Destroy();
		void Update(float deltaTime_ms);
		void Draw(float deltaTime_ms);

		void SetComboDisplayState(HitEvaluation hitEvaluation, u32 combo, vec2& position);
		void SetScoreBonusDisplayState(u32 value, vec2& position);
		void HoldScoreBonus();
		void ReleaseScoreBonus(bool drop);
	private:
		Context& mainGameContext;

		struct Implementation;
		Implementation* implementation{ nullptr };
	};
}
