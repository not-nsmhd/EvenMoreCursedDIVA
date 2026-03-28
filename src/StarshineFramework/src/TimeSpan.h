#pragma once
#include "Common/Types.h"

namespace Starshine
{
	struct TimeSpan
	{
		TimeSpan() {};

		constexpr TimeSpan(i64 microseconds) : Microseconds(microseconds) {};

		inline f64 GetMilliseconds() const { return static_cast<f64>(Microseconds) / (1000.0 * 1.0); };
		inline f64 GetSeconds() const { return static_cast<f64>(Microseconds) / (1000.0 * 1000.0); };
		
		inline TimeSpan& operator+=(const TimeSpan& other) { Microseconds += other.Microseconds; return *this; };
		inline TimeSpan& operator-=(const TimeSpan& other) { Microseconds -= other.Microseconds; return *this; };

		i64 Microseconds{ 0 };
	};

	namespace TimeSpanConversion
	{
		inline constexpr TimeSpan FromMilliseconds(f64 milliseconds) { return TimeSpan(static_cast<i64>(milliseconds * 1000.0)); };
		inline constexpr TimeSpan FromSeconds(f64 seconds) { return TimeSpan(static_cast<i64>(seconds * 1000000.0)); };
	}

	struct GameTime
	{
		TimeSpan ElapsedFrameTime{};
		TimeSpan TargetFrameTime{};

		TimeSpan TimeSinceLaunch{};
	};

	inline TimeSpan& operator+(const TimeSpan& a, const TimeSpan& b)
	{
		TimeSpan result{ a.Microseconds + b.Microseconds };
		return result;
	};

	inline TimeSpan& operator-(const TimeSpan& a, const TimeSpan& b)
	{
		TimeSpan result{ a.Microseconds - b.Microseconds };
		return result;
	};

	inline bool operator>(const TimeSpan& a, const TimeSpan& b) { return a.Microseconds > b.Microseconds; };
	inline bool operator>=(const TimeSpan& a, const TimeSpan& b) { return a.Microseconds >= b.Microseconds; };
	inline bool operator<(const TimeSpan& a, const TimeSpan& b) { return a.Microseconds < b.Microseconds; };
	inline bool operator<=(const TimeSpan& a, const TimeSpan& b) { return a.Microseconds <= b.Microseconds; };
}
