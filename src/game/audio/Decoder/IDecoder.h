#pragma once
#include "common/types.h"

namespace Starshine::Audio
{
	struct DecoderOutputData
	{
		u32 ChannelCount = 0;
		u32 SampleRate = 0;
		size_t SampleCount = 0;
		i16* Samples = nullptr;
	};

	class IDecoder
	{
	public:
		virtual ~IDecoder() = default;

		virtual bool TryDecodeData(const void* fileData, size_t fileSize, DecoderOutputData& output) = 0;
	};
}
