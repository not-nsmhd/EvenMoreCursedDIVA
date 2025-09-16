#include "MemorySampleProvider.h"
#include <SDL2/SDL.h>
#include "util/logging.h"

namespace Starshine::Audio
{
	MemorySampleProvider::MemorySampleProvider()
	{
	}

	MemorySampleProvider::~MemorySampleProvider()
	{
		FreeSamples();
	}

	void MemorySampleProvider::FreeSamples()
	{
		if (samples != nullptr)
		{
			delete[] samples;
			samples = nullptr;
		}
	}

	size_t MemorySampleProvider::GetSamples(i16* dstBuffer, size_t sampleOffset, size_t sampleCount)
	{
		assert(dstBuffer != nullptr);
		i16* copySource = &samples[sampleOffset];
		size_t copyAmount = std::min(this->sampleCount - sampleOffset, sampleCount);

		SDL_memcpy(dstBuffer, copySource, copyAmount * sizeof(i16));

		return copyAmount;
	}

	size_t MemorySampleProvider::GetNextSamples(i16* dstBuffer, size_t sampleCount)
	{
		assert(dstBuffer != nullptr);
		i16* copySource = &samples[samplePosition];
		size_t copyAmount = std::min(this->sampleCount - samplePosition, sampleCount);

		SDL_memcpy(dstBuffer, copySource, copyAmount * sizeof(i16));
		samplePosition += copyAmount;

		return copyAmount;
	}

	void MemorySampleProvider::SeekSamples(size_t sampleOffset)
	{
		if (sampleOffset >= sampleCount)
		{
			return;
		}

		samplePosition = sampleOffset;
	}

	size_t MemorySampleProvider::GetSampleCount() const
	{
		return sampleCount;
	}

	u32 MemorySampleProvider::GetChannelCount() const
	{
		return channelCount;
	}

	u32 MemorySampleProvider::GetSampleRate() const
	{
		return sampleRate;
	}

	size_t MemorySampleProvider::GetSamplePosition() const
	{
		return samplePosition;
	}
}
