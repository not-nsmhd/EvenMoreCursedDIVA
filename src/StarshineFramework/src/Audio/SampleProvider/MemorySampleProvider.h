#pragma once
#include "ISampleProvider.h"

namespace Starshine::Audio
{
	class MemorySampleProvider : public ISampleProvider, NonCopyable
	{
		friend class AudioEngine;
		friend class DecoderFactory;

	public:
		MemorySampleProvider();
		~MemorySampleProvider();

		void Destroy();
		bool IsStreamingOnly() const;

		u32 GetChannelCount() const;
		u32 GetSampleRate() const;
		size_t GetSampleAmount() const;

		size_t GetLoopStart_Frames() const;
		size_t GetLoopEnd_Frames() const;

		size_t ReadSamples(i16* dstBuffer, size_t offset, size_t size);

	public:
		size_t GetSamplePosition() const;

		size_t GetNextSamples(i16* dstBuffer, size_t size);
		void Seek(size_t samplePosition);

	private:
		u32 channels{};
		u32 sampleRate{};

		size_t sampleCount{};
		i16* samples{};

		size_t samplePosition{};
		size_t loopStart_frames{};
		size_t loopEnd_frames{};
	};
}
