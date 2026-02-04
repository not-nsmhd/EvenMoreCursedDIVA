#include "StreamingSampleProvider.h"
#include <vorbis/vorbisfile.h>
#include "Common/Logging/Logging.h"

namespace Starshine::Audio
{
	namespace Vorbisfile
	{
		struct DecoderState
		{
			std::unique_ptr<u8[]> EncodedData{};
			size_t EncodedDataSize{};
			size_t EncodedPosition{};
		};

		static size_t VF_Read(void* dst, size_t itemAmount, size_t itemSize, void* decoderState)
		{
			DecoderState* state = static_cast<DecoderState*>(decoderState);
			size_t readSize = itemAmount * itemSize;

			readSize = SDL_min(readSize, state->EncodedDataSize - state->EncodedPosition);

			if (readSize == 0)
			{
				return 0;
			}

			std::copy(&state->EncodedData[state->EncodedPosition], &state->EncodedData[state->EncodedPosition + readSize], reinterpret_cast<u8*>(dst));
			state->EncodedPosition += readSize;
			return readSize;
		}

		static int VF_Seek(void* decoderState, ogg_int64_t offset, int dir)
		{
			DecoderState* state = static_cast<DecoderState*>(decoderState);
			switch (dir)
			{
			case SEEK_SET:
				state->EncodedPosition = offset;
				return 0;
			case SEEK_CUR:
				state->EncodedPosition += offset;
				return 0;
			case SEEK_END:
				state->EncodedPosition = state->EncodedDataSize - offset - 1;
				return 0;
			}

			return -1;
		}

		static int VF_Close(void* decoderState)
		{
			return 0;
		}

		static long VF_Tell(void* decoderState)
		{
			DecoderState* state = static_cast<DecoderState*>(decoderState);
			return static_cast<long>(state->EncodedPosition);
		}

		static constexpr ov_callbacks VF_Callbacks
		{
			VF_Read,
			VF_Seek,
			VF_Close,
			VF_Tell
		};

		static std::string GetVorbisPropertyValue(const char* comment, int commentLength, std::string_view name)
		{
			if (name.compare(0, name.length(), comment, name.length()) == 0)
			{
				return std::string(&comment[name.length() + 1], commentLength - name.length() - 1);
			}
			return "";
		}
	}

	struct StreamingSampleProvider::Impl
	{
		using DecoderState = Vorbisfile::DecoderState;

		u32 channels{};
		u32 sampleRate{};

		size_t sampleCount{};

		size_t samplePosition{};
		size_t loopStart_frames{};
		size_t loopEnd_frames{};

		DecoderState state{};

		OggVorbis_File ovFile{};
		int currentBitstream{};

		bool Initialize(const u8* encodedData, size_t encodedDataSize)
		{
			state.EncodedData = std::make_unique<u8[]>(encodedDataSize);
			std::copy(&encodedData[0], &encodedData[encodedDataSize - 1], &state.EncodedData[0]);
			state.EncodedDataSize = encodedDataSize;

			if (ov_open_callbacks(&state, &ovFile, NULL, 0, Vorbisfile::VF_Callbacks) != 0)
			{
				return false;
			}

			vorbis_info* info = ov_info(&ovFile, -1);
			vorbis_comment* comments = ov_comment(&ovFile, -1);

			std::string commentValue{};
			if (comments != nullptr)
			{
				for (int i = 0; i < comments->comments; i++)
				{
					char* commentData = comments->user_comments[i];
					int commentLength = comments->comment_lengths[i];

					commentValue = Vorbisfile::GetVorbisPropertyValue(commentData, commentLength, "LoopStart");
					if (commentValue.length() > 0)
					{
						loopStart_frames = std::stoull(commentValue);
						continue;
					}

					commentValue = Vorbisfile::GetVorbisPropertyValue(commentData, commentLength, "LoopEnd");
					if (commentValue.length() > 0)
					{
						loopEnd_frames = std::stoull(commentValue);
						continue;
					}
				}
			}

			channels = info->channels;
			sampleRate = info->rate;
			sampleCount = ov_pcm_total(&ovFile, -1) * info->channels;

			return true;
		}

		void Destroy()
		{
			ov_clear(&ovFile);
		}

		size_t ReadNextSamples(i16* dstBuffer, size_t size)
		{
			size_t remainingSamples = sampleCount - samplePosition;
			size_t samplesToRead = SDL_min(remainingSamples, size);

			if (samplesToRead > 0)
			{
				long targetBytes = samplesToRead * static_cast<long>(sizeof(i16));

				long totalReadBytes = 0;
				long remainingBytes = targetBytes;
				size_t copyOffset = 0;

				while (remainingBytes > 0)
				{
					long readBytes = ov_read(&ovFile, &reinterpret_cast<char*>(dstBuffer)[copyOffset], remainingBytes, 0, 2, 1, &currentBitstream);
					copyOffset += readBytes;
					totalReadBytes += readBytes;
					remainingBytes -= readBytes;
				}

				samplePosition += totalReadBytes / sizeof(i16);

				return static_cast<size_t>(totalReadBytes / sizeof(i16));
			}

			return samplesToRead;
		}

		void Seek(size_t position)
		{
			if (position < sampleCount)
			{
				ov_pcm_seek(&ovFile, position / channels);
				samplePosition = position;
			}
		}
	};

	StreamingSampleProvider::StreamingSampleProvider(const u8* encodedData, size_t encodedDataSize)
	{
		impl = std::make_unique<Impl>();
		impl->Initialize(encodedData, encodedDataSize);
	}

	StreamingSampleProvider::~StreamingSampleProvider()
	{
	}

	void StreamingSampleProvider::Destroy()
	{
		impl->Destroy();
	}

	bool StreamingSampleProvider::IsStreamingOnly() const
	{
		return true;
	}

	u32 StreamingSampleProvider::GetChannelCount() const
	{
		return impl->channels;
	}

	u32 StreamingSampleProvider::GetSampleRate() const
	{
		return impl->sampleRate;
	}

	size_t StreamingSampleProvider::GetSampleAmount() const
	{
		return impl->sampleCount;
	}

	size_t StreamingSampleProvider::GetLoopStart_Frames() const
	{
		return impl->loopStart_frames;
	}

	size_t StreamingSampleProvider::GetLoopEnd_Frames() const
	{
		return impl->loopEnd_frames;
	}

	size_t StreamingSampleProvider::ReadSamples(i16* dstBuffer, size_t offset, size_t size)
	{
		return 0;
	}

	size_t StreamingSampleProvider::GetSamplePosition() const
	{
		return impl->samplePosition;
	}

	size_t StreamingSampleProvider::GetNextSamples(i16* dstBuffer, size_t size)
	{
		return impl->ReadNextSamples(dstBuffer, size);
	}

	void StreamingSampleProvider::Seek(size_t samplePosition)
	{
		impl->Seek(samplePosition);
	}
}
