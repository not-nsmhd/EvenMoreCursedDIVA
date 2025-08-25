#pragma once
#include "common/types.h"

namespace Starshine::Audio
{
	class ISampleProvider
	{
	public:
		virtual ~ISampleProvider() = default;

		virtual size_t GetSamples(i16* dstBuffer, size_t sampleOffset, size_t sampleCount) = 0;
		virtual size_t GetSampleCount() const = 0;

		virtual u32 GetChannelCount() const = 0;
		virtual u32 GetSampleRate() const = 0;
	};
}
