#pragma once
#include "../common/types.h"
#include "main_game.h"
#include "hit_evaluation.h"

namespace MainGame
{
	class HUD : NonCopyable
	{
	public:
		HUD(Context& context) : mainGameContext{ context } {}

		void Initialize();
		void Destroy();
		void Update(float deltaTime_ms);
		void Draw(float deltaTime_ms);

		void SetComboDisplayState(HitEvaluation hitEvaluation, u32 combo, vec2& position);
	private:
		Context& mainGameContext;

		struct Implementation;
		Implementation* implementation{ nullptr };
	};
}
