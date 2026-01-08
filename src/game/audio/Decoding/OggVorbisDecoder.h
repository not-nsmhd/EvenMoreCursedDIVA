#pragma once
#include "common/types.h"
#include "IDecoder.h"

namespace Starshine::Audio
{
	class OggVorbisDecoder : public IDecoder
	{
	public:
		const char* GetFileExtension() const override;
		bool ParseEncodedData(const void* encodedData, size_t encodedDataSize, DecoderOutput& output) override;
	};
}
