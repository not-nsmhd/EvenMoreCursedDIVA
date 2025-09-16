#pragma once
#include "common/types.h"

namespace Starshine::Audio
{
	class ISampleProvider
	{
	public:
		virtual ~ISampleProvider() = default;
		virtual void FreeSamples() = 0;

		virtual size_t GetSamples(i16* dstBuffer, size_t sampleOffset, size_t sampleCount) = 0;
		virtual size_t GetNextSamples(i16* dstBuffer, size_t sampleCount) = 0;
		virtual void SeekSamples(size_t sampleOffset) = 0;
		virtual size_t GetSampleCount() const = 0;

		virtual u32 GetChannelCount() const = 0;
		virtual u32 GetSampleRate() const = 0;
		virtual size_t GetSamplePosition() const = 0;
	};
}
