#pragma once
#include "common/types.h"

namespace Starshine::Audio
{
	struct DecoderOutput
	{
		u32 ChannelCount{};
		u32 SampleRate{};

		size_t SampleCount{};
		i16* SampleData{};

		bool IsLooped{};
		size_t LoopStart{};
		size_t LoopEnd{};
	};

	class WavDecoder
	{
	public:
		bool ParseEncodedData(const void* encodedData, size_t encodedDataSize, DecoderOutput& output);
	};
}
