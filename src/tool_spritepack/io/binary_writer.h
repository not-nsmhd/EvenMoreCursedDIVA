#pragma once
#include <iostream>
#include "../int_types.h"

enum class SeekOrigin
{
	BEGIN,
	CURRENT,
	END
};

constexpr u8 PaddingBytes[8] = { 'H', 'A', 'W', 'K', 'T', 'U', 'A', 'H' };

class BinaryWriter
{
public:
	BinaryWriter();
	BinaryWriter(std::iostream* baseStream, size_t alignment);
	
	size_t GetLocation();
	size_t GetSize();

	void Write(const void* src, size_t size);
	void Seek(SeekOrigin origin, size_t offset);

	void WriteU8(u8 value);
	void WriteU16(u16 value);
	void WriteU32(u32 value);
	void WriteU64(u64 value);
	void WriteI8(i8 value);
	void WriteI16(i16 value);
	void WriteI32(i32 value);
	void WriteI64(i64 value);

	void WriteFloat(float value);
	void WriteDouble(double value);

	void WriteLengthPrefixedString(std::string value);
	void Pad(size_t size);
	void PadUntilAligned();
private:
	std::iostream* baseStream = nullptr;
	size_t location = 0;
	size_t size = 0;
	size_t alignment = 1;
};
