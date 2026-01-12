#include "StreamReader.h"

namespace Starshine::IO
{
	std::string StreamReader::ReadString()
	{
		size_t length = static_cast<size_t>(ReadU32());
		if (length == 0 || length > GetSize())
		{
			return "";
		}

		std::string result = std::string(length, 0);
		ReadBuffer(result.data(), length);
		return result;
	}

	std::string StreamReader::ReadStringPointer()
	{
		size_t strPtr = ReadPointer();
		return ReadValueAt<std::string>(strPtr, [](StreamReader& reader)
		{
			return reader.ReadString();
		});
	}

	void StreamReader::SkipUntilAligned(i32 alignment, bool force)
	{
		i32 pos = static_cast<i32>(GetPosition());
		i32 bytesUntilAligned = ((pos + (alignment - 1)) & ~(alignment - 1)) - pos;

		if (bytesUntilAligned <= 0 && force)
		{
			bytesUntilAligned = alignment;
		}

		if (bytesUntilAligned > 0)
		{
			Skip(bytesUntilAligned);
		}
	}

	void StreamReader::OnPointerSizeChange()
	{
		switch (pointerSize)
		{
		case PointerSize::Size32Bit:
			readPtrFunc = &StreamReader::ReadPointer_32;
			readSizeFunc = &StreamReader::ReadSize_32;
			return;
		case PointerSize::Size64Bit:
			readPtrFunc = &StreamReader::ReadPointer_64;
			readSizeFunc = &StreamReader::ReadSize_64;
			return;
		}
	}

	void StreamReader::OnEndianessChange()
	{
		switch (endianess)
		{
		case Endianess::Little:
			readU16func = &StreamReader::ReadU16_LE;
			readU32func = &StreamReader::ReadU32_LE;
			readU64func = &StreamReader::ReadU64_LE;
						   				 
			readI16func = &StreamReader::ReadI16_LE;
			readI32func = &StreamReader::ReadI32_LE;
			readI64func = &StreamReader::ReadI64_LE;
						   				 
			readF32func = &StreamReader::ReadF32_LE;
			readF64func = &StreamReader::ReadF64_LE;
			return;
		case Endianess::Big:
			// TODO: Implement
			return;
		}
	}
}
