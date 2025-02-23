#pragma once
#include "../../common/int_types.h"

namespace GFX
{
	namespace LowLevel
	{
		enum class BufferType
		{
			BUFFER_NONE = -1,
			BUFFER_VERTEX,
			BUFFER_INDEX
		};

		enum class BufferUsage
		{
			BUFFER_USAGE_NONE = -1,
			BUFFER_USAGE_STATIC,
			BUFFER_USAGE_DYNAMIC
		};

		enum class BufferMapping
		{
			BUFFER_MAPPING_READ,
			BUFFER_MAPPING_WRITE,
			BUFFER_MAPPING_READWRITE
		};

		enum class IndexFormat
		{
			INDEX_16BIT,
			INDEX_32BIT
		};

		class Buffer
		{
		public:
			Buffer() {}

			virtual u32 GetSize() const = 0;
			virtual BufferType GetType() const = 0;
		};
	}
}
