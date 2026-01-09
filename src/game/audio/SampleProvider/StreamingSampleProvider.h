#pragma once
#include "ISampleProvider.h"
#include <memory>

namespace Starshine::Audio
{
	class StreamingSampleProvider : public ISampleProvider, NonCopyable
	{
		friend class AudioEngine;
		friend class DecoderFactory;

	public:
		StreamingSampleProvider(const u8* encodedData, size_t encodedDataSize);
		~StreamingSampleProvider();

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
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
