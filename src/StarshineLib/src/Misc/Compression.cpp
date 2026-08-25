#include "Compression.h"
#include "Common/Logging/Logging.h"
#include "Common/MathExt.h"
#include <SDL2/SDL_stdinc.h>
#include "bzlib.h"

using namespace Starshine;

extern "C" void bz_internal_error(int errcode)
{
	LogError("bzip2", "Internal error\nerrcode = %d", errcode);
}

namespace Starshine::Misc::Compression
{
	static constexpr i32 MaxBlockReadSize = 4096;
	static constexpr const char* LogName = "Starshine::Misc::Compression";

	size_t CompressData(const u8* data, size_t dataSize, IO::IStream& outputStream, i32 blockSize_100k)
	{
		assert(outputStream.IsWritable());
		assert(data != nullptr && dataSize > 0);

		bz_stream bzStream{};
		int result = BZ_OK;
		if (result = BZ2_bzCompressInit(&bzStream, blockSize_100k, 0, 0), result != BZ_OK)
		{
			LogError("bzip2", "Failed to initialize bzip2 compression stream. Error: %d", result);
			return 0;
		}

		char inputBlock[MaxBlockReadSize]{};
		char compressedBlock[MaxBlockReadSize]{};

		i32 inputDataPos{};
		i32 remainingDataSize = static_cast<i32>(dataSize); // TODO: Handle 64-bit sizes properly

		size_t compressedSize = 0;

		while (remainingDataSize > 0)
		{
			i32 availIn = MathExtensions::Min(remainingDataSize, MaxBlockReadSize);
			SDL_memcpy(inputBlock, &data[inputDataPos], availIn);
			bzStream.avail_in = availIn;
			bzStream.next_in = inputBlock;

			while (true)
			{
				bzStream.avail_out = MaxBlockReadSize;
				bzStream.next_out = compressedBlock;

				int state = BZ2_bzCompress(&bzStream, BZ_RUN);
				if (state != BZ_RUN_OK)
				{
					LogError("bzip2", "BZ2_bzCompress returned %d. Input data position: %d", state, inputDataPos);
					return 0;
				}

				if (bzStream.avail_out < MaxBlockReadSize)
				{
					size_t writeSize = MaxBlockReadSize - bzStream.avail_out;
					outputStream.WriteBuffer(compressedBlock, writeSize);
					compressedSize += writeSize;
				}

				if (bzStream.avail_in == 0)
					break;
			}

			inputDataPos += availIn;
			remainingDataSize -= availIn;
		}

		// finish up
		while (true)
		{
			bzStream.avail_out = MaxBlockReadSize;
			bzStream.next_out = compressedBlock;

			int state = BZ2_bzCompress(&bzStream, BZ_FINISH);

			size_t writeSize = MaxBlockReadSize - bzStream.avail_out;
			outputStream.WriteBuffer(compressedBlock, writeSize);
			compressedSize += writeSize;

			if (state == BZ_STREAM_END)
				break;
		}

		BZ2_bzCompressEnd(&bzStream);

		return compressedSize;
	}

	void DecompressStream(IO::IStream& inputStream, size_t compressedDataSize, u8* decompressedData)
	{
		assert(inputStream.IsReadable());
		assert(decompressedData != nullptr);

		bz_stream bzStream{};
		int result = BZ_OK;
		if (result = BZ2_bzDecompressInit(&bzStream, 0, 0), result != BZ_OK)
		{
			LogError("bzip2", "Failed to initialize bzip2 decompression stream. Error: %d", result);
			return;
		}

		size_t maxSize = (compressedDataSize == 0) ? inputStream.GetSize() : compressedDataSize;

		char inputBlock[MaxBlockReadSize]{};
		char decompressedBlock[MaxBlockReadSize]{};
		size_t decompressedPos{};

		while (true)
		{
			if (bzStream.avail_in == 0)
			{
				bzStream.avail_in = inputStream.ReadBuffer(inputBlock, MaxBlockReadSize);
				bzStream.next_in = inputBlock;
			}

			bzStream.avail_out = MaxBlockReadSize;
			bzStream.next_out = decompressedBlock;

			int state = BZ2_bzDecompress(&bzStream);
			if (state != BZ_OK && state != BZ_STREAM_END)
			{
				LogError("bzip2", "BZ2_bzDecompress returned %d. Compressed stream position: %d", state, bzStream.total_in_lo32);
				return;
			}

			size_t copySize = MaxBlockReadSize - bzStream.avail_out;
			SDL_memcpy(&decompressedData[decompressedPos], decompressedBlock, copySize);
			decompressedPos += copySize;

			if (state == BZ_STREAM_END)
				break;
			else if (bzStream.total_in_lo32 > maxSize)
			{
				LogError(LogName, "End of stream reached before BZ_STREAM_END. Compressed stream position: %d/%llu", bzStream.total_in_lo32, maxSize);
				return;
			}
		}

		BZ2_bzDecompressEnd(&bzStream);
	}

	void DecompressStream(IO::IStream& inputStream, size_t compressedDataSize, IO::IStream& outputStream)
	{
		assert(inputStream.IsReadable());
		assert(outputStream.IsWritable());

		bz_stream bzStream{};
		int result = BZ_OK;
		if (result = BZ2_bzDecompressInit(&bzStream, 0, 0), result != BZ_OK)
		{
			LogError("bzip2", "Failed to initialize bzip2 decompression stream. Error: %d", result);
			return;
		}

		size_t maxSize = (compressedDataSize == 0) ? inputStream.GetSize() : compressedDataSize;

		char inputBlock[MaxBlockReadSize]{};
		char decompressedBlock[MaxBlockReadSize]{};
		size_t decompressedPos{};

		while (true)
		{
			if (bzStream.avail_in == 0)
			{
				bzStream.avail_in = inputStream.ReadBuffer(inputBlock, MaxBlockReadSize);
				bzStream.next_in = inputBlock;
			}

			bzStream.avail_out = MaxBlockReadSize;
			bzStream.next_out = decompressedBlock;

			int state = BZ2_bzDecompress(&bzStream);
			if (state != BZ_OK && state != BZ_STREAM_END)
			{
				LogError("bzip2", "BZ2_bzDecompress returned %d. Compressed stream position: %d", state, bzStream.total_in_lo32);
				return;
			}

			size_t copySize = MaxBlockReadSize - bzStream.avail_out;
			outputStream.WriteBuffer(decompressedBlock, copySize);

			if (state == BZ_STREAM_END)
				break;
			else if (bzStream.total_in_lo32 > maxSize)
			{
				LogError(LogName, "End of stream reached before BZ_STREAM_END. Compressed stream position: %d/%llu", bzStream.total_in_lo32, maxSize);
				return;
			}
		}

		BZ2_bzDecompressEnd(&bzStream);
	}
}
