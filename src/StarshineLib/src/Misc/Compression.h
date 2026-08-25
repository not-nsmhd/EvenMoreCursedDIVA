#pragma once
#include "Common/Types.h"
#include "IO/IStream.h"

namespace Starshine::Misc
{
	namespace Compression
	{
		constexpr i32 DefaultBlockSize_100k = 5;

		size_t CompressData(const u8* data, size_t dataSize, IO::IStream& outputStream,
			i32 blockSize_100k = DefaultBlockSize_100k);

		void DecompressStream(IO::IStream& inputStream, size_t compressedDataSize, u8* decompressedData);
		void DecompressStream(IO::IStream& inputStream, size_t compressedDataSize, IO::IStream& outputStream);
	}
}
