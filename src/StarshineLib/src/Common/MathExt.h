#pragma once
#include "Types.h"
#include <type_traits>

namespace Starshine::MathExtensions
{
	constexpr f32 PiOver2 = 3.1415927f / 2.0f;
	constexpr f32 PiOver4 = 3.1415927f / 4.0f;
	constexpr f32 Pi = 3.1415927f;
	constexpr f32 TwoPi = 6.2831854f;

	constexpr vec2 RotateVector(vec2 point, vec2 origin, f32 cos, f32 sin)
	{
		vec2 p = point - origin;

		f32 x = p.x * cos - p.y * sin;
		f32 y = p.x * sin + p.y * cos;

		return { x, y };
	};

	constexpr vec2 RotateVector(vec2 point, vec2 origin, f32 radians)
	{
		return RotateVector(point, origin, glm::cos(radians), glm::sin(radians));
	};

	constexpr f32 ToRadians(f32 degrees)
	{
		return glm::radians<f32>(degrees);
	}
		
	constexpr f32 ToDegrees(f32 radians)
	{
		return glm::degrees<f32>(radians);
	}

	constexpr vec2 AbsVector2(vec2 point)
	{
		return vec2(glm::abs(point.x), glm::abs(point.y));
	}

	constexpr vec2 GetSinePoint(f64 percentage, vec2 target, f32 deg, f32 freq, f32 ampl, f32 dist)
	{
		if (dist == 0.0f)
		{
			return target;
		}

		if (std::fmodf(freq, 2.0f) != 0.0f)
		{
			freq *= -1.0f;
		}

		f64 x = percentage * dist;
		f64 y = glm::sin(percentage * Pi * freq) / 12.0f * ampl;
		f32 radians = glm::radians<f32>(deg - 90.0f);

		vec2 point = RotateVector(vec2(x, y), vec2(0.0f), radians);
		return point + target;
	}

	template <typename N>
	constexpr bool IsPowerOf2(N value)
	{
		// https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
		static_assert(std::is_integral_v<N>);
		return (value & (value - 1)) == 0;
	};

	template <typename N>
	constexpr N NearestPowerOf2(N value)
	{
		// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		static_assert(std::is_integral_v<N>);
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		return ++value;
	};

	constexpr f32 CalculateBarDuration_Seconds(f32 bpm, int beatsPerBar)
	{
		if (bpm <= 0.0f || beatsPerBar <= 0)
		{
			return 0.0f;
		}

		return (60.0f / bpm * static_cast<f32>(beatsPerBar));
	}

	constexpr bool ApproxiamtelyEqual(f32 a, f32 b, f32 range = 0.0001f) { return glm::abs(a - b) < range; };

	template <typename T> constexpr T Min(T a, T b) { return (a < b) ? a : b; };
	template <typename T> constexpr T Max(T a, T b) { return (a > b) ? a : b; };
	template <typename T> constexpr T Clamp(T value, T min, T max) { return Min<T>(Max<T>(value, min), max); };

	template <typename T>
	constexpr bool IsInRange(T start, T end, T value)
	{
		static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>);
		return (value >= start && value <= end);
	}

	template <typename T>
	constexpr T ConvertRange(T originalStart, T originalEnd, T newStart, T newEnd, T value)
	{
		static_assert(std::is_floating_point_v<T>);
		return newStart + (value - originalStart) * (newEnd - newStart) / (originalEnd - originalStart);
	}
};
