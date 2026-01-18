#pragma once
#include "Common/Types.h"

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

	// NOTE: All values are specified in milliseconds
	namespace HitThresholds
	{
		constexpr f64 CoolThreshold = 30.0;
		constexpr f64 GoodThreshold = 70.0;
		constexpr f64 SafeThreshold = 100.0;
		constexpr f64 BadThreshold = 130.0;
				  
		constexpr f64 ThresholdStart = 130.0;
		constexpr f64 ThresholdMiss = -130.0;
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
