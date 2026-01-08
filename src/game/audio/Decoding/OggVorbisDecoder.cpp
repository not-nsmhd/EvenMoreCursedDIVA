#include "OggVorbisDecoder.h"
#include <vorbis/vorbisfile.h>
#include <SDL2/SDL_stdinc.h>
#include <array>

namespace Starshine::Audio
{
	static constexpr size_t VorbisfileBufferSize = 4096;

	namespace Vorbisfile
	{
		struct DecoderState
		{
			const void* EncodedData{};
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

			SDL_memcpy(dst, &(reinterpret_cast<const u8*>(state->EncodedData))[state->EncodedPosition], readSize);
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
	}

	static std::string GetVorbisPropertyValue(const char* comment, int commentLength, std::string_view name)
	{
		if (name.compare(0, name.length(), comment, name.length()) == 0)
		{
			return std::string(&comment[name.length() + 1], commentLength - name.length() - 1);
		}
		return "";
	}

	const char* OggVorbisDecoder::GetFileExtension() const
	{
		return ".ogg";
	}

	bool OggVorbisDecoder::ParseEncodedData(const void* encodedData, size_t encodedDataSize, DecoderOutput& output)
	{
		if (encodedData != nullptr && encodedDataSize > 0)
		{
			Vorbisfile::DecoderState decoderState{ encodedData, encodedDataSize, 0 };
			OggVorbis_File ovFile{};

			if (ov_open_callbacks(&decoderState, &ovFile, NULL, 0, Vorbisfile::VF_Callbacks) != 0)
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

					commentValue = GetVorbisPropertyValue(commentData, commentLength, "LoopStart");
					if (commentValue.length() > 0)
					{
						output.LoopStart = std::stoull(commentValue);
						continue;
					}

					commentValue = GetVorbisPropertyValue(commentData, commentLength, "LoopEnd");
					if (commentValue.length() > 0)
					{
						output.LoopEnd = std::stoull(commentValue);
						output.IsLooped = true;
						continue;
					}
				}
			}

			output.ChannelCount = info->channels;
			output.SampleRate = info->rate;
			output.SampleCount = ov_pcm_total(&ovFile, -1) * info->channels;
			output.SampleData = new i16[output.SampleCount];

			std::array<char, VorbisfileBufferSize> buffer;
			int currentBitstream = -1;
			size_t copyOffset = 0;

			while (copyOffset < (output.SampleCount * sizeof(i16)))
			{
				long readSize = ov_read(&ovFile, &buffer[0], VorbisfileBufferSize, 0, 2, 1, &currentBitstream);
				if (readSize == 0)
				{
					break;
				}

				// HACK: The casting from i16 to u8 array is needed since the
				//		 readSize is the amount of BYTES read and its value may not be divisible by 2
				//		 (it's also the same reason why the SampleCount variable is multiplied by 2 in the while loop condition)
				SDL_memcpy(&(reinterpret_cast<u8*>(output.SampleData))[copyOffset], &buffer[0], readSize);
				copyOffset += readSize;
			}

			ov_clear(&ovFile);
			return true;
		}

		return false;
	}
}
