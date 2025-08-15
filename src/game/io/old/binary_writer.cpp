#include "binary_writer.h"

BinaryWriter::BinaryWriter()
{
}

BinaryWriter::BinaryWriter(std::iostream* baseStream, size_t alignment)
{
	this->baseStream = baseStream;
	this->alignment = alignment;	
}

size_t BinaryWriter::GetLocation()
{
	return location;
}

size_t BinaryWriter::GetSize()
{
	return size;
}

void BinaryWriter::Write(const void* src, size_t size)
{
	if (src != nullptr)
	{
		baseStream->write((char*)src, size);
		location += size;
		if (location > size)
		{
			this->size = location;
		}
	}
}

void BinaryWriter::Seek(SeekOrigin origin, size_t offset)
{
	switch (origin)
	{
		case SeekOrigin::Begin:
			location = offset;
			break;
		case SeekOrigin::Current:
			location += offset;
			break;
		case SeekOrigin::End:
			location = size - offset - 1;
			break;
	}
}

void BinaryWriter::WriteU8(u8 value)
{
	Write(&value, sizeof(u8));
}

void BinaryWriter::WriteU16(u16 value)
{
	Write(&value, sizeof(u16));
}

void BinaryWriter::WriteU32(u32 value)
{
	Write(&value, sizeof(u32));
}

void BinaryWriter::WriteU64(u64 value)
{
	Write(&value, sizeof(u64));
}

void BinaryWriter::WriteI8(i8 value)
{
	Write(&value, sizeof(i8));
}

void BinaryWriter::WriteI16(i16 value)
{
	Write(&value, sizeof(i16));
}

void BinaryWriter::WriteI32(i32 value)
{
	Write(&value, sizeof(i32));
}

void BinaryWriter::WriteI64(i64 value)
{
	Write(&value, sizeof(i64));
}

void BinaryWriter::WriteFloat(float value)
{
	Write(&value, sizeof(float));
}

void BinaryWriter::WriteDouble(double value)
{
	Write(&value, sizeof(double));
}

void BinaryWriter::WriteLengthPrefixedString(std::string value)
{
	u32 stringLength = value.length();
	WriteU32(stringLength);
	Write(&value[0], stringLength);
}

void BinaryWriter::Pad(size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		WriteU8(0);
	}
}

void BinaryWriter::PadUntilAligned()
{
	if (alignment == 0)
	{
		return;
	}

	size_t paddingSize = alignment - (location % alignment);
	Pad(paddingSize);
}
