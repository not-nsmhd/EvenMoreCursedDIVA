#include <SDL2/SDL.h>
#include "StreamingSampleProvider.h"
#include "util/logging.h"

namespace Starshine::Audio
{
	using namespace Logging;

	constexpr const char* LogName = "Starshine::Audio::StreamingSampleProvider";

	namespace VorbisfileCallbacks
	{
		size_t Read(void* dst, size_t elementSize, size_t amount, void* sampleProviderPtr)
		{
			StreamingSampleProvider* sampleProvider = static_cast<StreamingSampleProvider*>(sampleProviderPtr);
			size_t readData = sampleProvider->ReadEncodedData(dst, elementSize * amount);

			return readData;
		}

		int Seek(void* sampleProviderPtr, ogg_int64_t offset, int dir)
		{
			StreamingSampleProvider* sampleProvider = static_cast<StreamingSampleProvider*>(sampleProviderPtr);
			size_t locationToSet = 0;

			switch (dir)
			{
			case SEEK_SET:
				locationToSet = offset;
				break;
			case SEEK_CUR:
				locationToSet = sampleProvider->GetEncodedDataPosition() + offset;
				break;
			case SEEK_END:
				locationToSet = (sampleProvider->GetEncodedDataSize()) - offset;
				break;
			}

			sampleProvider->SeekEncodedData(locationToSet);

			return 0;
		}

		long Tell(void* sampleProviderPtr)
		{
			StreamingSampleProvider* sampleProvider = static_cast<StreamingSampleProvider*>(sampleProviderPtr);
			size_t position = sampleProvider->GetEncodedDataPosition();

			return static_cast<long>(position);
		}

		ov_callbacks Callbacks =
		{
			&Read,
			&Seek,
			NULL,
			&Tell
		};
	}

	StreamingSampleProvider::StreamingSampleProvider(size_t encodedDataSize, u8* encodedData)
		: encodedDataSize(encodedDataSize), encodedData(encodedData)
	{
		OpenVorbisfile();
	}

	StreamingSampleProvider::~StreamingSampleProvider()
	{
		FreeSamples();
	}

	void StreamingSampleProvider::OpenVorbisfile()
	{
		int res = ov_open_callbacks(this, &ovFile, NULL, 0, VorbisfileCallbacks::Callbacks);

		vorbis_info* info = ov_info(&ovFile, -1);
		channelCount = info->channels;
		sampleRate = info->rate;
		sampleCount = ov_pcm_total(&ovFile, -1);
	}

	void StreamingSampleProvider::FreeSamples()
	{
		ov_clear(&ovFile);
		delete[] encodedData;
	}

	size_t StreamingSampleProvider::GetSamples(i16* dstBuffer, size_t sampleOffset, size_t sampleCount)
	{
		return 0;
	}

	size_t StreamingSampleProvider::GetNextSamples(i16* dstBuffer, size_t sampleCount)
	{
		int bytesToDecodeCount = static_cast<int>(std::min(this->sampleCount - samplePosition, sampleCount) * sizeof(i16)) * channelCount;
		long decodedBytesCount = ov_read(&ovFile, reinterpret_cast<char*>(decodedDataBuffer), bytesToDecodeCount, 0, 2, 1, &currentBitstream);

		if (decodedBytesCount == 0)
		{
			return 0;
		}
		else if (decodedBytesCount < 0)
		{
			LogError(LogName, "Vorbis decoding error %d at PCM position %llu (VF offset: %llu)", decodedBytesCount, samplePosition, ovFile.offset);
			return 0;
		}

		SDL_memcpy(dstBuffer, decodedDataBuffer, decodedBytesCount);
		samplePosition += static_cast<size_t>(decodedBytesCount) / sizeof(i16);

		return static_cast<size_t>(decodedBytesCount) / sizeof(i16);
	}

	void StreamingSampleProvider::SeekSamples(size_t sampleOffset)
	{
		samplePosition = sampleOffset;
		ov_pcm_seek(&ovFile, sampleOffset);
	}

	size_t StreamingSampleProvider::GetSampleCount() const
	{
		return sampleCount;
	}

	u32 StreamingSampleProvider::GetChannelCount() const
	{
		return channelCount;
	}

	u32 StreamingSampleProvider::GetSampleRate() const
	{
		return sampleRate;
	}

	size_t StreamingSampleProvider::GetSamplePosition() const
	{
		return samplePosition;
	}

	size_t StreamingSampleProvider::ReadEncodedData(void* dstBuffer, size_t size)
	{
		u8* copyPosition = encodedData + encodedDataPosition;
		size_t copyAmount = std::min(encodedDataSize - encodedDataPosition, size);

		if (copyAmount == 0)
		{
			return 0;
		}

		SDL_memcpy(dstBuffer, copyPosition, copyAmount);

		encodedDataPosition += copyAmount;
		return copyAmount;
	}

	size_t StreamingSampleProvider::GetEncodedDataPosition() const
	{
		return encodedDataPosition;
	}

	size_t StreamingSampleProvider::GetEncodedDataSize() const
	{
		return encodedDataSize;
	}

	void StreamingSampleProvider::SeekEncodedData(size_t location)
	{
		encodedDataPosition = location;
	}
}
