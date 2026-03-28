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
		constexpr TimeSpan CoolThreshold = TimeSpanConversion::FromMilliseconds(30.0);
		constexpr TimeSpan GoodThreshold = TimeSpanConversion::FromMilliseconds(70.0);
		constexpr TimeSpan SafeThreshold = TimeSpanConversion::FromMilliseconds(100.0);
		constexpr TimeSpan BadThreshold = TimeSpanConversion::FromMilliseconds(130.0);
				  
		constexpr TimeSpan ThresholdStart = TimeSpanConversion::FromMilliseconds(130.0);
		constexpr TimeSpan ThresholdMiss = TimeSpanConversion::FromMilliseconds(-130.0);
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
