#pragma once
#include "Common/Types.h"
#include "pcg/pcg_variants.h"

namespace Starshine::Random
{
	class RNG : NonCopyable
	{
	public:
		RNG();
		RNG(i64 seed);
		~RNG() = default;

	public:
		u32 Next(u32 max = 0);
		f64 NextDouble(bool getSigned);

	private:
		pcg32_random_t rngState{};
	};
}
