#pragma once
#include "Common/Types.h"

using Starshine::TimeSpan;
using namespace Starshine;

namespace DIVA::MainGame
{
	enum class HitEvaluation : u8
	{
		None,

		Cool,
		Good,
		Safe,
		Bad,
		Miss,

		Count
	};

	namespace HitThresholds
	{
		constexpr TimeSpan CoolThreshold = TimeSpan::FromMilliseconds(30.0);
		constexpr TimeSpan GoodThreshold = TimeSpan::FromMilliseconds(70.0);
		constexpr TimeSpan SafeThreshold = TimeSpan::FromMilliseconds(100.0);
		constexpr TimeSpan BadThreshold = TimeSpan::FromMilliseconds(130.0);
				  
		constexpr TimeSpan ThresholdStart = TimeSpan::FromMilliseconds(130.0);
		constexpr TimeSpan ThresholdMiss = TimeSpan::FromMilliseconds(-130.0);
	}

	namespace ScoreValues
	{
		constexpr u32 Cool = 500;
		constexpr u32 Good = 300;
		constexpr u32 Safe = 100;
		constexpr u32 Bad = 50;

		constexpr u32 CoolWrong = 250;
		constexpr u32 GoodWrong = 150;
		constexpr u32 SafeWrong = 50;
		constexpr u32 BadWrong = 30;

		constexpr u32 DoubleBonus = 200;
	}
}
