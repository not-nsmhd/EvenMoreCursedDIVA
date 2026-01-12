#pragma once
#include "StreamManipulator.h"
#include <vector>

namespace Starshine::IO
{
	class StreamReader : public StreamManipulator, NonCopyable
	{
	public:
		explicit StreamReader(IStream& stream) : StreamManipulator(stream)
		{
			assert(stream.IsReadable());
			OnEndianessChange();
			OnPointerSizeChange();
		}

		~StreamReader() = default;

	public:
		inline size_t ReadBuffer(void* dst, size_t size) { return baseStream->ReadBuffer(dst, size); }

		std::string ReadString();
		std::string ReadStringPointer();

		template<typename Func>
		void ReadAt(size_t position, Func func)
		{
			size_t returnPos = GetPosition();

			Seek(position);
			func(*this);
			Seek(returnPos);
		}

		template<typename T, typename Func>
		T ReadValueAt(size_t position, Func func)
		{
			size_t returnPos = GetPosition();

			Seek(position);
			const T value = func(*this);
			Seek(returnPos);

			return value;
		}

		void SkipUntilAligned(i32 alignment, bool force = false);

	public:
		inline u8 ReadU8() { return ReadType_NativeOrder<u8>(); }
		inline u16 ReadU16() { return (this->*readU16func)(); }
		inline u32 ReadU32() { return (this->*readU32func)(); }
		inline u64 ReadU64() { return (this->*readU64func)(); }

		inline i8 ReadI8() { return ReadType_NativeOrder<i8>(); }
		inline i16 ReadI16() { return (this->*readI16func)(); }
		inline i32 ReadI32() { return (this->*readI32func)(); }
		inline i64 ReadI64() { return (this->*readI64func)(); }

		inline f32 ReadF32() { return (this->*readF32func)(); }
		inline f64 ReadF64() { return (this->*readF64func)(); }

		inline size_t ReadPointer() { return (this->*readPtrFunc)(); }
		inline size_t ReadSize() { return (this->*readSizeFunc)(); }

	public:
		template<typename T>
		T ReadType_NativeOrder() { T value{}; ReadBuffer(&value, sizeof(T)); return value; }

		inline u16 ReadU16_LE() { return ReadType_NativeOrder<u16>(); }
		inline u32 ReadU32_LE() { return ReadType_NativeOrder<u32>(); }
		inline u64 ReadU64_LE() { return ReadType_NativeOrder<u64>(); }

		inline i16 ReadI16_LE() { return ReadType_NativeOrder<i16>(); }
		inline i32 ReadI32_LE() { return ReadType_NativeOrder<i32>(); }
		inline i64 ReadI64_LE() { return ReadType_NativeOrder<i64>(); }

		inline f32 ReadF32_LE() { return ReadType_NativeOrder<f32>(); }
		inline f64 ReadF64_LE() { return ReadType_NativeOrder<f64>(); }

		inline size_t ReadPointer_32() { return static_cast<size_t>(ReadU32()); }
		inline size_t ReadSize_32() { return static_cast<size_t>(ReadU32()); };

		inline size_t ReadPointer_64() { return static_cast<size_t>(ReadU64()); }
		inline size_t ReadSize_64() { return static_cast<size_t>(ReadU64()); };

	protected:
		void OnPointerSizeChange() override;
		void OnEndianessChange() override;

	private:
		u16 (StreamReader::*readU16func)() { nullptr };
		u32 (StreamReader::*readU32func)() { nullptr };
		u64 (StreamReader::*readU64func)() { nullptr };

		i16 (StreamReader::*readI16func)() { nullptr };
		i32 (StreamReader::*readI32func)() { nullptr };
		i64 (StreamReader::*readI64func)() { nullptr };

		f32 (StreamReader::*readF32func)() { nullptr };
		f64 (StreamReader::*readF64func)() { nullptr };

		size_t (StreamReader::*readPtrFunc)() { nullptr };
		size_t (StreamReader::*readSizeFunc)() { nullptr };
	};
}
