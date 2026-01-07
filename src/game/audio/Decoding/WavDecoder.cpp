#include "WavDecoder.h"
#include <SDL2/SDL_rwops.h>
#include <array>

namespace Starshine::Audio
{
	using WavFileID = std::array<char, 4>;
	using ID3FrameID = std::array<char, 4>;

	static constexpr WavFileID RiffHeaderID { 'R', 'I', 'F', 'F' };
	static constexpr WavFileID WaveTypeID { 'W', 'A', 'V', 'E' };
	static constexpr WavFileID FormatSegmentID { 'f', 'm', 't', ' ' };
	static constexpr WavFileID DataSegmentID { 'd', 'a', 't', 'a' };
	static constexpr WavFileID ID3MetadataSegmentID { 'i', 'd', '3', ' ' };

	static constexpr ID3FrameID ID3LoopFrameID { 'T', 'X', 'X', 'X' };

	struct WavSegment
	{
		WavFileID Name{};
		i32 Size{};
	};

	// NOTE: no way we're doing compiler-specific hacks already
#ifdef _MSC_VER
__pragma (pack(push, 1))
#endif // _MSC_VER
	struct ID3Frame
	{
		ID3FrameID Name{};
		i32 Size{};
		i16 Flags{};
	};
#ifdef _MSC_VER
__pragma (pack(pop))
#endif // _MSC_VER

	// NOTE: https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatex
	struct WavFormat
	{
		i16 DataFormat{}; // NOTE: Must be 1 (WAVE_FORMAT_PCM)
		u16 Channels{};
		u32 SamplesPerSecond{};
		u32 AvgBytesPerSec{};
		u16 BlockAlign{};
		u16 BitsPerSample{};
	};

	void ReadID3Frame(SDL_RWops* stream, ID3Frame& frame)
	{
		SDL_RWread(stream, &frame.Name, sizeof(ID3FrameID), 1);
		frame.Size = SDL_ReadBE32(stream);
		frame.Flags = SDL_ReadBE16(stream);
	}

	std::string ReadNullTerminatedString(SDL_RWops* stream, size_t maxLength)
	{
		size_t readLength = 0;
		size_t readPos = SDL_RWtell(stream);

		while (SDL_RWtell(stream) < SDL_RWsize(stream))
		{
			char c = SDL_ReadU8(stream);

			if (c == 0 || readLength + 1 > maxLength)
			{
				break;
			}

			readLength++;
		}

		if (readLength == 0)
		{
			return "";
		}

		std::string result = std::string(readLength, '\0');

		SDL_RWseek(stream, readPos, RW_SEEK_SET);
		SDL_RWread(stream, result.data(), readLength, 1);
		
		if (readLength < maxLength)
		{
			SDL_RWseek(stream, 1, RW_SEEK_CUR);
		}

		return result;
	}

	bool WavDecoder::ParseEncodedData(const void* encodedData, size_t encodedDataSize, DecoderOutput& output)
	{
		if (encodedData != nullptr && encodedDataSize > 0)
		{
			SDL_RWops* memStream = SDL_RWFromConstMem(encodedData, static_cast<int>(encodedDataSize));

			// --- Initial RIFF Header

			WavSegment riffHeader{};
			memStream->read(memStream, &riffHeader, sizeof(WavSegment), 1);

			if (riffHeader.Name != RiffHeaderID)
			{
				SDL_RWclose(memStream);
				return false;
			}

			WavFileID riffDataType{};
			memStream->read(memStream, &riffDataType, sizeof(WavFileID), 1);

			if (riffDataType != WaveTypeID)
			{
				SDL_RWclose(memStream);
				return false;
			}

			// --- Format Segment
			WavSegment formatSegment{};
			memStream->read(memStream, &formatSegment, sizeof(WavSegment), 1);

			if (formatSegment.Name != FormatSegmentID)
			{
				SDL_RWclose(memStream);
				return false;
			}

			WavFormat audioFormat{};
			memStream->read(memStream, &audioFormat, sizeof(WavFormat), 1);

			// --- Data Segment

			WavSegment dataSegment{};
			memStream->read(memStream, &dataSegment, sizeof(WavSegment), 1);

			if (dataSegment.Name != DataSegmentID)
			{
				SDL_RWclose(memStream);
				return false;
			}

			output.ChannelCount = audioFormat.Channels;
			output.SampleRate = audioFormat.SamplesPerSecond;
			output.SampleCount = static_cast<size_t>(dataSegment.Size) / sizeof(i16);
			output.SampleData = new i16[output.SampleCount];

			memStream->read(memStream, output.SampleData, output.SampleCount * sizeof(i16), 1);

			// --- ID3 Metadata (used for looping, optional)

			size_t remainingFileSize = encodedDataSize - (44ULL + dataSegment.Size);
			size_t metadataStartPos = SDL_RWtell(memStream) + sizeof(WavSegment);

			if (remainingFileSize > 0)
			{
				WavSegment metadataSegment{};
				memStream->read(memStream, &metadataSegment, sizeof(WavSegment), 1);
				SDL_RWclose(memStream);

				// NOTE: https://id3.org/id3v2.3.0
				if (metadataSegment.Name == ID3MetadataSegmentID)
				{
					SDL_RWops* id3Stream = SDL_RWFromConstMem(&((u8*)encodedData)[metadataStartPos], metadataSegment.Size);

					if (SDL_ReadBE32(id3Stream) != 0x49443303) // 'I', 'D', '3', 0x03
					{
						// Oh well, no custom looping position for us
						SDL_RWclose(id3Stream);
						return true;
					}

					SDL_RWseek(id3Stream, 5, RW_SEEK_CUR); // Second version byte + flags (unused)
					u8 metadataSize = SDL_ReadU8(id3Stream);

					if (metadataSegment.Size - SDL_RWtell(id3Stream) < metadataSize)
					{
						SDL_RWclose(id3Stream);
						return true;
					}

					// --- Loop Start
					ID3Frame id3Frame{};
					ReadID3Frame(id3Stream, id3Frame);

					SDL_ReadU8(id3Stream); // Encoding

					std::string frameName = ReadNullTerminatedString(id3Stream, id3Frame.Size);
					if (frameName != "LoopStart")
					{
						SDL_RWclose(id3Stream);
						return true;
					}

					std::string frameValue = ReadNullTerminatedString(id3Stream, id3Frame.Size - (frameName.length() + 2));
					output.LoopStart = std::stoull(frameValue);

					// --- Loop End
					ReadID3Frame(id3Stream, id3Frame);

					SDL_ReadU8(id3Stream); // Encoding

					frameName = ReadNullTerminatedString(id3Stream, id3Frame.Size);
					if (frameName != "LoopEnd")
					{
						output.LoopStart = 0;
						SDL_RWclose(id3Stream);
						return true;
					}

					frameValue = ReadNullTerminatedString(id3Stream, id3Frame.Size - (frameName.length() + 2));
					output.LoopEnd = std::stoull(frameValue);
					output.IsLooped = true;
				}
			}

			return true;
		}

		return false;
	}
}
