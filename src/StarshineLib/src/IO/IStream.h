#pragma once
#include "Common/Types.h"

namespace Starshine::IO
{
	class IStream
	{
	public:
		virtual ~IStream() = default;

	public:
		virtual void Close() = 0;

		virtual bool IsOpen() const = 0;
		virtual bool IsReadable() const = 0;
		virtual bool IsWritable() const = 0;

		virtual size_t GetPosition() const = 0;
		virtual size_t GetSize() const = 0;
		virtual size_t Seek(size_t position) = 0;

		virtual size_t ReadBuffer(void* dest, size_t size) = 0;
		virtual size_t WriteBuffer(const void* src, size_t size) = 0;
	};
}
