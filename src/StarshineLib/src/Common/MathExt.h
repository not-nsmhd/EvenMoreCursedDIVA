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
			return 0.0f;

		return (60.0f / bpm * static_cast<f32>(beatsPerBar));
	}

	constexpr bool ApproxiamtelyEqual(f32 a, f32 b, f32 range = 0.0001f) { return glm::abs(a - b) < range; };

	template <typename T> constexpr T Min(T a, T b) { return (a < b) ? a : b; };
	template <typename T> constexpr T Max(T a, T b) { return (a > b) ? a : b; };
	template <typename T> constexpr T Clamp(T value, T min, T max) { return Min<T>(Max<T>(value, min), max); };

	template <typename T>
	constexpr bool IsInRange(T start, T end, T value)
	{
		return (value >= start && value <= end);
	}

	template <typename T>
	constexpr T ConvertRange(T originalStart, T originalEnd, T newStart, T newEnd, T value)
	{
		return newStart + (value - originalStart) * (newEnd - newStart) / (originalEnd - originalStart);
	}

	template <typename T>
	constexpr T Lerp(T start, T end, f32 factor)
	{
		return (1.0f - factor) * start + factor * end;
	}

	constexpr size_t GetAlignedSize(size_t size, size_t alignment)
	{
		return (size + (alignment - 1)) & ~(alignment - 1);
	}

	constexpr f32 BezierSlopeFromT(f32 a, f32 b, f32 c, f32 t)
	{
		const f32 tSquared = t * t;
		return 1.0f / (3.0f * a * tSquared + 2.0f * b * t + c);
	}

	constexpr void GetCubicBezierPolynomials(const f32& start, const f32& end, f32 output[4])
	{
		output[0] = 1.0f - 3.0f * end + 3.0f * start - 0.0f;
		output[1] = 3.0f * end - 6.0f * start + 3.0f * 0.0f;
		output[2] = 3.0f * start - 3.0f * 0.0f;
		output[3] = 0.0f;
	}

	constexpr f32 GetCubicBezierPoint(f32 start, f32 cp0, f32 cp1, f32 end, f32 t)
	{
		const f32 tSquared = t * t;
		const f32 tCubed = t * t * t;

		return start * tCubed + cp0 * tSquared + cp1 * t + end;
	}

	constexpr f32 GetEasingPoint(const vec2& start, const vec2& end, const f32& t)
	{
		// NOTE: stolen from https://gist.github.com/iinfin/0f1e782dd868e4ee8716e385634a930e
		if (start == end)
			return t;

		f32 plTime[4]{};
		f32 plValue[4]{};

		GetCubicBezierPolynomials(start.x, end.x, plTime);
		GetCubicBezierPolynomials(start.y, end.y, plValue);

		f32 current = t;

		f32 currentX = GetCubicBezierPoint(plTime[0], plTime[1], plTime[2], plTime[3], current);
		f32 currentSlope = BezierSlopeFromT(plTime[0], plTime[1], plTime[2], current);

		if (currentSlope == INFINITY)
			currentSlope = t;

		current -= (currentX - t) * currentSlope;
		current = MathExtensions::Min(MathExtensions::Max(current, 0.0f), 1.0f);

		return GetCubicBezierPoint(plValue[0], plValue[1], plValue[2], plValue[3], current);
	}
};
