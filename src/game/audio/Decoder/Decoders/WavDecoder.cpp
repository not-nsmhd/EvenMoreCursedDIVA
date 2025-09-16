#include "WavDecoder.h"
#include "util/logging.h"
#include <SDL2/SDL_audio.h>

namespace Starshine::Audio
{
	constexpr const char* LogName = "Starshine::Audio::WavDecoder";

	bool WavDecoder::TryDecodeData(const void* fileData, size_t fileSize, DecoderOutputData& output)
	{
		assert(fileSize <= std::numeric_limits<int>::max());
		SDL_RWops* wavFile = SDL_RWFromConstMem(fileData, static_cast<int>(fileSize));
		
		if (wavFile == nullptr)
		{
			Logging::LogError(LogName, "Failed to create SDL_RWops from provided pointer. Error: %s", SDL_GetError());
			return false;
		}

		SDL_AudioSpec audioSpec{};
		u8* audioBytes = nullptr;
		u32 audioLength = 0;
		if (SDL_LoadWAV_RW(wavFile, 0, &audioSpec, &audioBytes, &audioLength) == nullptr)
		{
			Logging::LogError(LogName, "Failed to parse WAV data from provided pointer. Error: %s", SDL_GetError());
			return false;
		}

		output.ChannelCount = static_cast<u32>(audioSpec.channels);
		output.SampleRate = static_cast<u32>(audioSpec.freq);
		output.SampleCount = static_cast<size_t>(audioLength) / sizeof(i16);
		output.Samples = new i16[output.SampleCount];

		SDL_memcpy(output.Samples, audioBytes, static_cast<size_t>(audioLength));
		SDL_FreeWAV(audioBytes);
		SDL_RWclose(wavFile);

		return true;
	}
}
