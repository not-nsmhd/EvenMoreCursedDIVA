#include "binary_reader.h"

BinaryReader::BinaryReader(std::iostream* baseStream, size_t alignment)
{
	this->baseStream = baseStream;
	this->alignment = alignment;	
}

size_t BinaryReader::GetLocation()
{
	return location;
}

size_t BinaryReader::GetSize()
{
	return size;
}

void BinaryReader::Read(void* output, size_t size)
{
	if (output != nullptr)
	{
		baseStream->read((char*)output, size);
		location += size;
		if (location > size)
		{
			this->size = location;
		}
	}
}

void BinaryReader::Seek(SeekOrigin origin, size_t offset)
{
	switch (origin)
	{
		case SeekOrigin::BEGIN:
			location = offset;
			break;
		case SeekOrigin::CURRENT:
			location += offset;
			break;
		case SeekOrigin::END:
			location = size - offset - 1;
			break;
	}
}

u8 BinaryReader::ReadU8()
{
	u8 value = 0;
	Read(&value, sizeof(u8));
	return value;
}

u16 BinaryReader::ReadU16()
{
	u16 value = 0;
	Read(&value, sizeof(u16));
	return value;
}

u32 BinaryReader::ReadU32()
{
	u32 value = 0;
	Read(&value, sizeof(u32));
	return value;
}

u64 BinaryReader::ReadU64()
{
	u64 value = 0;
	Read(&value, sizeof(u64));
	return value;
}

i8 BinaryReader::ReadI8()
{
	i8 value = 0;
	Read(&value, sizeof(i8));
	return value;
}

i16 BinaryReader::ReadI16()
{
	i16 value = 0;
	Read(&value, sizeof(i16));
	return value;
}

i32 BinaryReader::ReadI32()
{
	i32 value = 0;
	Read(&value, sizeof(i32));
	return value;
}

i64 BinaryReader::ReadI64()
{
	i64 value = 0;
	Read(&value, sizeof(i64));
	return value;
}

float BinaryReader::ReadFloat()
{
	float value = 0;
	Read(&value, sizeof(float));
	return value;
}

double BinaryReader::ReadDouble()
{
	double value = 0;
	Read(&value, sizeof(double));
	return value;
}

std::string BinaryReader::ReadLengthPrefixedString()
{
	u32 stringLength = ReadU32();
	std::string value = std::string(stringLength, 'A');

	Read(&value[0], stringLength);
	return value;
}

void BinaryReader::SkipPadding(size_t size)
{
	Seek(SeekOrigin::CURRENT, size);
}

void BinaryReader::SkipPaddingUntilAligned()
{
	if (alignment == 0)
	{
		return;
	}

	size_t paddingSize = alignment - (location % alignment);
	SkipPadding(paddingSize);
}
