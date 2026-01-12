#pragma once
#include "Common/Types.h"
#include "IStream.h"
#include "BinaryMode.h"

namespace Starshine::IO
{
	class StreamManipulator
	{
	public:
		StreamManipulator(IStream& stream) : baseStream{ &stream } {};
		~StreamManipulator() = default;

	public:
		inline size_t GetPosition() { return baseStream->GetPosition(); }
		inline size_t GetSize() { return baseStream->GetSize(); }
		inline size_t GetRemainingSize() { return baseStream->GetSize() - baseStream->GetPosition(); }
		inline bool EndOfFile() { return baseStream->GetPosition() >= baseStream->GetSize(); }

		inline void Seek(size_t position) { baseStream->Seek(position); }
		inline void Skip(size_t offset) { baseStream->Seek(baseStream->GetPosition() + offset); }

		inline PointerSize GetPointerSize() { return pointerSize; };
		inline void SetPointerSize(PointerSize ptrSize) { pointerSize = ptrSize; OnPointerSizeChange(); };

		inline Endianess GetEndianess() { return endianess; };
		inline void SetEndianess(Endianess endianess) { this->endianess = endianess; OnEndianessChange(); };

	protected:
		virtual void OnPointerSizeChange() = 0;
		virtual void OnEndianessChange() = 0;

	protected:
		PointerSize pointerSize{ PointerSize::Size32Bit };
		Endianess endianess{ Endianess::Little };

		IStream* baseStream{};
	};
}
