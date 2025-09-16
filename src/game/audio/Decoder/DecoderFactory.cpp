#include "DecoderFactory.h"
#include "Decoders/WavDecoder.h"
#include "audio/SampleProvider/MemorySampleProvider.h"
#include "io/File.h"

namespace Starshine::Audio
{
	using namespace IO;

	DecoderFactory* decoderFactoryInstance = nullptr;

	DecoderFactory::DecoderFactory()
	{
		wavDecoder = new WavDecoder();
	}

	ISampleProvider* DecoderFactory::DecodeFile(std::string_view filePath)
	{
		u8* fileData = nullptr;
		size_t fileSize = File::ReadAllBytes(filePath, &fileData);

		if (fileSize == 0 || fileData == nullptr)
		{
			return nullptr;
		}

		ISampleProvider* sampleProvider = DecodeData(fileData, fileSize);
		delete[] fileData;

		if (sampleProvider == nullptr)
		{
			return nullptr;
		}

		return sampleProvider;
	}

	ISampleProvider* DecoderFactory::DecodeData(const void* fileData, size_t fileSize)
	{
		if (fileData == nullptr || fileSize == 0)
		{
			return nullptr;
		}

		DecoderOutputData outputData = {};
		if (wavDecoder->TryDecodeData(fileData, fileSize, outputData) == false)
		{
			return nullptr;
		}

		MemorySampleProvider* sampleProvider = new MemorySampleProvider();
		sampleProvider->channelCount = outputData.ChannelCount;
		sampleProvider->sampleRate = outputData.SampleRate;
		sampleProvider->sampleCount = outputData.SampleCount;
		sampleProvider->samples = outputData.Samples;

		return sampleProvider;
	}

	DecoderFactory& DecoderFactory::GetInstance()
	{
		if (decoderFactoryInstance == nullptr)
		{
			decoderFactoryInstance = new DecoderFactory();
		}

		return *decoderFactoryInstance;
	}
}
