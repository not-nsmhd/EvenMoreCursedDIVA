#pragma once
#include "common/types.h"
#include "audio/SampleProvider/ISampleProvider.h"
#include "audio/Decoder/IDecoder.h"

namespace Starshine::Audio
{
	class DecoderFactory : NonCopyable
	{
	public:
		DecoderFactory();
		~DecoderFactory() = default;

	public:
		// NOTE: Returns a pointer to a sample provider with allocated sample data.
		// Both must be freed using "delete" when done using them.
		ISampleProvider* DecodeFile(std::string_view filePath);

		// NOTE: Returns a pointer to a sample provider with allocated sample data.
		// Both must be freed using "delete" when done using them.
		ISampleProvider* DecodeData(const void* fileData, size_t fileSize);

	public:
		static DecoderFactory& GetInstance();

	private:
		IDecoder* wavDecoder = nullptr;
	};
}
