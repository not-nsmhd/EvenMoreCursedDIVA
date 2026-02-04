#pragma once
#include "IDecoder.h"

namespace Starshine::Audio
{
	class WavDecoder : public IDecoder
	{
	public:
		const char* GetFileExtension() const override;
		bool ParseEncodedData(const void* encodedData, size_t encodedDataSize, DecoderOutput& output) override;
	};
}
