#include "MemorySampleProvider.h"
#include <SDL2/SDL.h>

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
		void* copySource = samples + sampleOffset;
		size_t copyAmount = std::min(this->sampleCount - sampleOffset, sampleCount);

		SDL_memcpy(dstBuffer, copySource, copyAmount * sizeof(i16));

		return copyAmount;
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
}
