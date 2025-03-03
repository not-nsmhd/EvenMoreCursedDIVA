#pragma once
#include "common/int_types.h"
#include <string>

namespace MainGame
{
	enum class HitValuation
	{
		NONE = -1,

		COOL,
		GOOD,
		SAFE,
		BAD,
		MISS,

		VALU_MAX
	};

	const std::string HitValuationNames[] = 
	{
		"Cool",
		"Good",
		"Safe",
		"Bad",
		"Miss"
	};

	extern u32 ScoreAmounts[5];

	class GameScore
	{
	public:
		GameScore();

		u32 GetCurrentScore();
		u32 GetCurrentCombo();
		u32 GetMaxCombo();

		void Reset();

		void AddScore(u32 amount);

		void IncrementCombo();
		void ResetCombo();

		void RegisterNoteHit(HitValuation valu, bool wrong);
	private:
		u32 currentScore = 0;
		u32 currentCombo = 0;
		u32 maxCombo = 0;
	};
}
