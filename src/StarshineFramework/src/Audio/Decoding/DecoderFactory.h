#pragma once
#include <vector>
#include "Common/Types.h"
#include "IDecoder.h"
#include "Audio/SampleProvider/ISampleProvider.h"

namespace Starshine::Audio
{
	class DecoderFactory : NonCopyable
	{
	public:
		DecoderFactory();
		~DecoderFactory();

	public:
		static DecoderFactory* GetInstance();
		static void DestoryInstance();

	public:
		ISampleProvider* DecodeFileData(std::string_view filename, const void* fileData, size_t fileSize);

	private:
		template <typename T>
		IDecoder* RegisterDecoder();

	private:
		std::vector<IDecoder*> decoders;
	};
}
