#pragma once
#include "Common/Types.h"

namespace Starshine
{
	struct TimeSpan
	{
	public:
		TimeSpan() {};
		constexpr TimeSpan(i64 microseconds) : Microseconds(microseconds) {};

		static constexpr TimeSpan Zero() { return TimeSpan(0); }
		static constexpr TimeSpan FromMilliseconds(f64 milliseconds) { return TimeSpan(static_cast<i64>(milliseconds * 1000.0)); };
		static constexpr TimeSpan FromSeconds(f64 seconds) { return TimeSpan(static_cast<i64>(seconds * 1000000.0)); };

	public:
		constexpr f64 GetMilliseconds() const { return static_cast<f64>(Microseconds) / (1000.0 * 1.0); };
		constexpr f64 GetSeconds() const { return static_cast<f64>(Microseconds) / (1000.0 * 1000.0); };
		
	public:
		constexpr TimeSpan& operator+=(const TimeSpan& other)	{ Microseconds += other.Microseconds; return *this; };
		constexpr TimeSpan& operator-=(const TimeSpan& other)	{ Microseconds -= other.Microseconds; return *this; };

		constexpr TimeSpan operator+(const TimeSpan other)	const { return TimeSpan(Microseconds + other.Microseconds); }
		constexpr TimeSpan operator-(const TimeSpan other)	const { return TimeSpan(Microseconds - other.Microseconds); }

		constexpr bool operator>(const TimeSpan& other)		const { return Microseconds > other.Microseconds; };
		constexpr bool operator>=(const TimeSpan& other)	const { return Microseconds >= other.Microseconds; };
		constexpr bool operator<(const TimeSpan& other)		const { return Microseconds < other.Microseconds; };
		constexpr bool operator<=(const TimeSpan& other)	const { return Microseconds <= other.Microseconds; };
		constexpr bool operator==(const TimeSpan& other)	const { return Microseconds == other.Microseconds; };
		constexpr bool operator!=(const TimeSpan& other)	const { return Microseconds != other.Microseconds; };

		constexpr TimeSpan operator-()	const { return TimeSpan(-Microseconds); };
		constexpr TimeSpan operator+()	const { return TimeSpan(-Microseconds); };

	public:
		static TimeSpan GetTimeNow();

		i64 Microseconds{ 0 };
	};

	struct GameTime
	{
		TimeSpan ElapsedFrameTime{};
		TimeSpan TargetFrameTime{};

		TimeSpan TimeSinceLaunch{};
	};
}
