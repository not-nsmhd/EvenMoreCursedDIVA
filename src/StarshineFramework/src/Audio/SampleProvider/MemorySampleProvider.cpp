#include "MemorySampleProvider.h"
#include "Common/Logging/Logging.h"
#include <SDL2/SDL_stdinc.h>

namespace Starshine::Audio
{
	MemorySampleProvider::MemorySampleProvider(const i16* sampleData, size_t sampleCount) : sampleCount(sampleCount)
	{
		samples = std::make_unique<u16[]>(sampleCount);
		std::copy(&sampleData[0], &sampleData[sampleCount - 1], samples.get());
	}

	MemorySampleProvider::~MemorySampleProvider()
	{
		Destroy();
	}

	void MemorySampleProvider::Destroy()
	{
		samples = nullptr;
	}

	bool MemorySampleProvider::IsStreamingOnly() const
	{
		return false;
	}

	u32 MemorySampleProvider::GetChannelCount() const
	{
		return channels;
	}

	u32 MemorySampleProvider::GetSampleRate() const
	{
		return sampleRate;
	}

	size_t MemorySampleProvider::GetSampleAmount() const
	{
		return sampleCount;
	}

	size_t MemorySampleProvider::GetLoopStart_Frames() const
	{
		return loopStart_frames;
	}

	size_t MemorySampleProvider::GetLoopEnd_Frames() const
	{
		return loopEnd_frames;
	}

	size_t MemorySampleProvider::ReadSamples(i16* dstBuffer, size_t offset, size_t size)
	{
		size_t remainingSamples = sampleCount - offset;
		size_t samplesToRead = SDL_min(remainingSamples, size);

		if (samplesToRead > 0)
		{
			SDL_memcpy(dstBuffer, &samples[offset], samplesToRead * sizeof(i16));
		}

		return samplesToRead;
	}

	size_t MemorySampleProvider::GetSamplePosition() const
	{
		return samplePosition;
	}

	size_t MemorySampleProvider::GetNextSamples(i16* dstBuffer, size_t size)
	{
		size_t remainingSamples = sampleCount - samplePosition;
		size_t samplesToRead = SDL_min(remainingSamples, size);

		if (samplesToRead > 0)
		{
			SDL_memcpy(dstBuffer, &samples[samplePosition], samplesToRead * sizeof(i16));
			samplePosition += samplesToRead;
		}

		return samplesToRead;
	}

	void MemorySampleProvider::Seek(size_t samplePosition)
	{
		if (samplePosition < sampleCount)
		{
			this->samplePosition = samplePosition;
		}
	}
}
