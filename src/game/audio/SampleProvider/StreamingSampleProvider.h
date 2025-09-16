#pragma once
#include "ISampleProvider.h"
#include <vorbisfile.h>

namespace Starshine::Audio
{
	class StreamingSampleProvider : public ISampleProvider, NonCopyable
	{
	public:
		StreamingSampleProvider(size_t encodedDataSize, u8* encodedData);
		~StreamingSampleProvider();

	public:
		void FreeSamples();

		size_t GetSamples(i16* dstBuffer, size_t sampleOffset, size_t sampleCount) override;
		size_t GetNextSamples(i16* dstBuffer, size_t sampleCount) override;
		void SeekSamples(size_t sampleOffset);
		size_t GetSampleCount() const override;

		u32 GetChannelCount() const override;
		u32 GetSampleRate() const override;
		size_t GetSamplePosition() const override;

		size_t ReadEncodedData(void* dstBuffer, size_t size);
		size_t GetEncodedDataPosition() const;
		size_t GetEncodedDataSize() const;
		void SeekEncodedData(size_t location);

	private:
		void OpenVorbisfile();

	private:
		u32 channelCount = 0;
		u32 sampleRate = 0;

		u8* encodedData = nullptr;
		size_t encodedDataSize = 0;
		size_t sampleCount = 0;

	private:
		OggVorbis_File ovFile{};

	private:
		i16 decodedDataBuffer[2048] = {};

		size_t samplePosition = 0;
		size_t encodedDataPosition = 0;

		int currentBitstream = 0;
	};
}
