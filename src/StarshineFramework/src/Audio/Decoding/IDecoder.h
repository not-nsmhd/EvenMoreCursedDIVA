#pragma once
#include "Common/Types.h"

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

	class IDecoder
	{
	public:
		virtual ~IDecoder() = default;

		virtual const char* GetFileExtension() const = 0;
		virtual bool ParseEncodedData(const void* encodedData, size_t encodedDataSize, DecoderOutput& output) = 0;
	};
}
