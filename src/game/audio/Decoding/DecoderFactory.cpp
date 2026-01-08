#include "DecoderFactory.h"
#include "WavDecoder.h"

#include "audio/SampleProvider/MemorySampleProvider.h"
#include <SDL2/SDL_stdinc.h>

namespace Starshine::Audio
{
	DecoderFactory* Instance = new DecoderFactory();

	DecoderFactory::DecoderFactory()
	{
		decoders.reserve(1);
		RegisterDecoder<WavDecoder>();
	}

	DecoderFactory::~DecoderFactory()
	{
		for (size_t i = 0; i < decoders.size(); i++)
		{
			delete decoders[i];
		}
		decoders.clear();
	}

	DecoderFactory* DecoderFactory::GetInstance()
	{
		return Instance;
	}

	void DecoderFactory::DestoryInstance()
	{
		delete[] Instance;
	}

	ISampleProvider* DecoderFactory::DecodeFileData(std::string_view filename, const void* fileData, size_t fileSize)
	{
		if (fileData == nullptr || fileSize == 0)
		{
			return nullptr;
		}

		for (auto& decoder : decoders)
		{
			DecoderOutput output{};
			if (decoder->ParseEncodedData(fileData, fileSize, output))
			{
				MemorySampleProvider* sampleProvider = new MemorySampleProvider();
				sampleProvider->samples = new i16[output.SampleCount];
				SDL_memcpy(sampleProvider->samples, output.SampleData, output.SampleCount * sizeof(i16));

				sampleProvider->sampleCount = output.SampleCount;
				sampleProvider->sampleRate = output.SampleRate;
				sampleProvider->channels = output.ChannelCount;

				if (output.IsLooped)
				{
					sampleProvider->loopStart_frames = output.LoopStart;
					sampleProvider->loopEnd_frames = output.LoopEnd;
				}
				else
				{
					sampleProvider->loopStart_frames = 0;
					sampleProvider->loopEnd_frames = output.SampleCount / output.ChannelCount;
				}

				return sampleProvider;
			}
		}

		return nullptr;
	}

	template<typename T>
	IDecoder* DecoderFactory::RegisterDecoder()
	{
		static_assert(std::is_base_of_v<IDecoder, T>, "T must be inhereited from IDecoder");
		return decoders.emplace_back(new T());
	}
}
