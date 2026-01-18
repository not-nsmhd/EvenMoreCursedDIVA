#pragma once
#include "StreamManipulator.h"
#include <vector>
#include <functional>

namespace Starshine::IO
{
	class StreamWriter : public StreamManipulator, NonCopyable
	{
	public:
		static constexpr u8 DefaultPaddingValue{ 0xEE };

	public:
		explicit StreamWriter(IStream& stream) : StreamManipulator(stream)
		{
			assert(stream.IsWritable());
			OnEndianessChange();
			OnPointerSizeChange();
		}

		~StreamWriter() = default;

	public:
		inline size_t WriteBuffer(const void* src, size_t size) { return baseStream->WriteBuffer(src, size); }

		void WriteString(std::string_view value);
		void WriteStringPointer(std::string_view value, i32 alignment = 0);

		void WriteFunctionPointer(const std::function<void(StreamWriter&)>& func);

		void WritePadding(size_t size, u8 value = DefaultPaddingValue);
		void WriteAlignedPadding(i32 alignment, bool force = false, u8 value = DefaultPaddingValue);

		void FlushStringArray();
		void FlushFunctionArray();

	public:
		inline void WriteU8(u8 value) { WriteType_NativeOrder<u8>(value); }
		inline void WriteU16(u16 value) { (this->*writeU16func)(value); }
		inline void WriteU32(u32 value) { (this->*writeU32func)(value); }
		inline void WriteU64(u64 value) { (this->*writeU64func)(value); }

		inline void WriteI8(i8 value) { WriteType_NativeOrder<i8>(value); }
		inline void WriteI16(i16 value) { (this->*writeI16func)(value); }
		inline void WriteI32(i32 value) { (this->*writeI32func)(value); }
		inline void WriteI64(i64 value) { (this->*writeI64func)(value); }

		inline void WriteF32(f32 value) { (this->*writeF32func)(value); }
		inline void WriteF64(f64 value) { (this->*writeF64func)(value); }

		inline void WritePointer(size_t value) { (this->*writePtrFunc)(value); }
		inline void WriteSize(size_t value) { (this->*writeSizeFunc)(value); }

	public:
		template<typename T>
		void WriteType_NativeOrder(T value) { WriteBuffer(&value, sizeof(T)); }

		inline void WriteU16_LE(u16 value) { WriteType_NativeOrder<u16>(value); }
		inline void WriteU32_LE(u32 value) { WriteType_NativeOrder<u32>(value); }
		inline void WriteU64_LE(u64 value) { WriteType_NativeOrder<u64>(value); }

		inline void WriteI16_LE(i16 value) { WriteType_NativeOrder<i16>(value); }
		inline void WriteI32_LE(i32 value) { WriteType_NativeOrder<i32>(value); }
		inline void WriteI64_LE(i64 value) { WriteType_NativeOrder<i64>(value); }

		inline void WriteF32_LE(f32 value) { WriteType_NativeOrder<f32>(value); }
		inline void WriteF64_LE(f64 value) { WriteType_NativeOrder<f64>(value); }

		inline void WritePointer_32(size_t value) { WriteU32(static_cast<u32>(value)); }
		inline void WriteSize_32(size_t value) { WriteU32(static_cast<u32>(value)); };

		inline void WritePointer_64(size_t value) { WriteU64(static_cast<u64>(value)); }
		inline void WriteSize_64(size_t value) { WriteU64(static_cast<u64>(value)); };

		// TODO: Implement big-endian writing functions

	protected:
		void OnPointerSizeChange() override;
		void OnEndianessChange() override;

	private:
		void (StreamWriter::*writeU16func)(u16) { nullptr };
		void (StreamWriter::*writeU32func)(u32) { nullptr };
		void (StreamWriter::*writeU64func)(u64) { nullptr };

		void (StreamWriter::*writeI16func)(i16) { nullptr };
		void (StreamWriter::*writeI32func)(i32) { nullptr };
		void (StreamWriter::*writeI64func)(i64) { nullptr };

		void (StreamWriter::*writeF32func)(f32) { nullptr };
		void (StreamWriter::*writeF64func)(f64) { nullptr };

		void (StreamWriter::* writePtrFunc)(size_t) { nullptr };
		void (StreamWriter::* writeSizeFunc)(size_t) { nullptr };

		struct StringPointerEntry
		{
			size_t PointerPosition{};
			std::string_view String;
			i32 Alignment{};
		};

		struct FunctionPointerEntry
		{
			size_t PointerPosition{};
			const std::function<void(StreamWriter&)> Function;
		};

		std::vector<StringPointerEntry> stringArray;
		std::vector<FunctionPointerEntry> functionArray;
	};
}
