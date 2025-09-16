#pragma once
#include "ISampleProvider.h"

namespace Starshine::Audio
{
	class MemorySampleProvider : public ISampleProvider, NonCopyable
	{
	public:
		MemorySampleProvider();
		~MemorySampleProvider();

	public:
		void FreeSamples();

		size_t GetSamples(i16* dstBuffer, size_t sampleOffset, size_t sampleCount) override;
		size_t GetNextSamples(i16* dstBuffer, size_t sampleCount) override;
		void SeekSamples(size_t sampleOffset) override;
		size_t GetSampleCount() const override;

		u32 GetChannelCount() const override;
		u32 GetSampleRate() const override;
		size_t GetSamplePosition() const override;

		u32 channelCount = 0;
		u32 sampleRate = 0;

		i16* samples = nullptr;
		size_t sampleCount = 0;

	private:
		size_t samplePosition = 0;
	};
}
