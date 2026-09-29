#include "RNG.h"
#include "Common/TimeSpan.h"
#include <cmath>

namespace Starshine::Random
{
	RNG::RNG()
	{
		pcg32_srandom_r(&rngState, TimeSpan::GetTimeNow().Microseconds, reinterpret_cast<u64>(&rngState));
	}

	RNG::RNG(i64 seed)
	{
		pcg32_srandom_r(&rngState, seed, reinterpret_cast<u64>(&rngState));
	}

	u32 RNG::Next(u32 max)
	{
		if (max == 0)
			return pcg32_random_r(&rngState);

		return pcg32_boundedrand_r(&rngState, max);
	}

	f64 RNG::NextDouble(bool isSigned)
	{
		f64 unsignedValue = ::ldexp(pcg32_random_r(&rngState), -32);
		if (isSigned)
			return (unsignedValue - 0.5f) * 2.0f;

		return unsignedValue;
	}
}
