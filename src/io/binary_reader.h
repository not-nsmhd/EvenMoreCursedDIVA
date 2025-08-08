#pragma once
#include <iostream>
#include "../common/int_types.h"

enum class SeekOrigin
{
	BEGIN,
	CURRENT,
	END
};

class BinaryReader
{
public:
	BinaryReader();
	BinaryReader(std::iostream* baseStream, size_t alignment);
	
	size_t GetLocation();
	size_t GetSize();

	void Read(void* output, size_t size);
	void Seek(SeekOrigin origin, size_t offset);

	u8 ReadU8();
	u16 ReadU16();
	u32 ReadU32();
	u64 ReadU64();
	i8 ReadI8();
	i16 ReadI16();
	i32 ReadI32();
	i64 ReadI64();

	float ReadFloat();
	double ReadDouble();

	std::string ReadLengthPrefixedString();
	void SkipPadding(size_t size);
	void SkipPaddingUntilAligned();
private:
	std::iostream* baseStream = nullptr;
	size_t location = 0;
	size_t size = 0;
	size_t alignment = 1;
};
