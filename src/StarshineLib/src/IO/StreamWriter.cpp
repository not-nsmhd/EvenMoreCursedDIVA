#include "StreamWriter.h"
#include <array>

namespace Starshine::IO
{
	void StreamWriter::WriteString(std::string_view value)
	{
		// Length-prefixed strings >>>>>>>>>>>>>>> Null-terminated strings
		WriteU32(static_cast<u32>(value.size()));
		if (value.size() > 0)
		{
			WriteBuffer(value.data(), value.size());
		}
	}

	void StreamWriter::WriteStringPointer(std::string_view value, i32 alignment)
	{
		if (value.empty())
		{
			WritePointer(0);
		}
		else
		{
			stringArray.push_back({ GetPosition(), value, alignment });
			WritePointer(0);
		}
	}

	void StreamWriter::WritePadding(size_t size, u8 value)
	{
		constexpr size_t maxPaddingSize = 32;
		assert(size <= maxPaddingSize);

		std::array<u8, maxPaddingSize> paddingBuffer;

		std::memset(paddingBuffer.data(), value, size);
		WriteBuffer(paddingBuffer.data(), size);
	}

	void StreamWriter::WriteAlignedPadding(i32 alignment, bool force, u8 value)
	{
		constexpr size_t maxAlignment = 32;
		assert(alignment <= maxAlignment);

		std::array<u8, maxAlignment> paddingBuffer;

		i32 curPos = static_cast<i32>(GetPosition());
		i32 paddingSize = ((curPos + (alignment - 1)) & ~(alignment - 1)) - curPos;

		if (force && paddingSize <= 0)
		{
			paddingSize = alignment;
		}

		std::memset(paddingBuffer.data(), value, paddingSize);
		WriteBuffer(paddingBuffer.data(), paddingSize);
	}

	void StreamWriter::FlushStringArray()
	{
		for (auto& str : stringArray)
		{
			size_t stringPosition = GetPosition();

			Seek(str.PointerPosition);
			WritePointer(stringPosition);

			Seek(stringPosition);
			WriteString(str.String);

			if (str.Alignment > 0)
			{
				WriteAlignedPadding(str.Alignment);
			}
		}
		stringArray.clear();
	}

	void StreamWriter::OnPointerSizeChange()
	{
		switch (pointerSize)
		{
		case PointerSize::Size32Bit:
			writePtrFunc = &StreamWriter::WritePointer_32;
			writeSizeFunc = &StreamWriter::WriteSize_32;
			return;
		case PointerSize::Size64Bit:
			writePtrFunc = &StreamWriter::WritePointer_64;
			writeSizeFunc = &StreamWriter::WriteSize_64;
			return;
		}
	}

	void StreamWriter::OnEndianessChange()
	{
		switch (endianess)
		{
		case Endianess::Little:
			writeU16func = &StreamWriter::WriteU16_LE;
			writeU32func = &StreamWriter::WriteU32_LE;
			writeU64func = &StreamWriter::WriteU64_LE;

			writeI16func = &StreamWriter::WriteI16_LE;
			writeI32func = &StreamWriter::WriteI32_LE;
			writeI64func = &StreamWriter::WriteI64_LE;

			writeF32func = &StreamWriter::WriteF32_LE;
			writeF64func = &StreamWriter::WriteF64_LE;
			return;
		case Endianess::Big:
			// TODO: Implement
			return;
		}
	}
}
