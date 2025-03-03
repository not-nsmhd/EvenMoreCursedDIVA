#include "gamescore.h"

namespace MainGame
{
	u32 ScoreAmounts[5] = 
	{
		500,
		300,
		100,
		50,
		0
	};

	GameScore::GameScore()
	{
	}
	
	u32 GameScore::GetCurrentScore()
	{
		return currentScore;
	}
	
	u32 GameScore::GetCurrentCombo()
	{
		return currentCombo;
	}
	
	u32 GameScore::GetMaxCombo()
	{
		return maxCombo;
	}
	
	void GameScore::Reset()
	{
		currentScore = 0;
		currentCombo = 0;
		maxCombo = 0;
	}
	
	void GameScore::AddScore(u32 amount)
	{
		currentScore += amount;
	}
	
	void GameScore::IncrementCombo()
	{
		currentCombo++;

		if (maxCombo < currentCombo)
		{
			maxCombo = currentCombo;
		}
	}
	
	void GameScore::ResetCombo()
	{
		currentCombo = 0;
	}
	
	void GameScore::RegisterNoteHit(HitValuation valu, bool wrong)
	{
		currentScore += ScoreAmounts[static_cast<int>(valu)] / (wrong ? 2 : 1);
		if (wrong || static_cast<int>(valu) >= static_cast<int>(HitValuation::SAFE))
		{
			currentCombo = 0;
		}
		else
		{
			IncrementCombo();
		}
	}
}