#pragma once
#include "common/types.h"

namespace MainGame
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
		constexpr f32 CoolThreshold = 30.0f;
		constexpr f32 GoodThreshold = 70.0f;
		constexpr f32 SafeThreshold = 100.0f;
		constexpr f32 BadThreshold = 130.0f;

		constexpr f32 ThresholdStart = 130.0f;
		constexpr f32 ThresholdMiss = -130.0f;
	}
}
