#pragma once
#include "types.h"
#include <type_traits>

namespace Common
{
	namespace MathExtensions
	{
		const float MATH_EXT_PI = 3.1415927f;
		const float MATH_EXT_2PI = 6.2831854f;

		vec2 RotateVector(vec2 point, vec2 origin, float cos, float sin);
		vec2 RotateVector(vec2 point, vec2 origin, float radians);
		float ToRadians(float degrees);
		float ToDegrees(float radians);

		vec2 AbsVector2(vec2 point);

		float ConvertRange(float originalStart, float originalEnd, float newStart, float newEnd, float value);
		vec2 GetSinePoint(float percentage, vec2 target, float deg, float freq, float ampl, float dist);

		template <typename T>
		constexpr bool IsInRange(T start, T end, T value) 
		{
			static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>);
			return (value >= start && value <= end);
		}

		template <typename N>
		constexpr bool IsPowerOf2(N value)
		{
			// https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
			static_assert(std::is_integral_v<N>);
			return (value & (value - 1)) == 0;
		};

		float CalculateBarDuration_Seconds(float bpm, int beatsPerBar);

		constexpr bool ApproxiamtelyEqual(float a, float b, float range = 0.0001f) { return glm::abs(a - b) < range; };
	};
};
