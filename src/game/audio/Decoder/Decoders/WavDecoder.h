#pragma once
#include "audio/Decoder/IDecoder.h"

namespace Starshine::Audio
{
	class WavDecoder : public IDecoder
	{
		bool TryDecodeData(const void* fileData, size_t fileSize, DecoderOutputData& output);
	};
}
