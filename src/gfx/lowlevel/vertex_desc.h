#pragma once
#include "../../common/int_types.h"

namespace GFX
{
	namespace LowLevel
	{
		enum class VertexAttributeType
		{
			VERT_ELEMENT_POSITION,
			VERT_ELEMENT_COLOR,
			VERT_ELEMENT_TEXCOORD,
			VERT_ELEMENT_NORMAL
		};

		enum class VertexAttributeFormat
		{
			VERT_FORMAT_BYTE,
			VERT_FORMAT_BYTE2,
			VERT_FORMAT_BYTE3,
			VERT_FORMAT_BYTE4,

			VERT_FORMAT_SHORT,
			VERT_FORMAT_SHORT2,
			VERT_FORMAT_SHORT3,
			VERT_FORMAT_SHORT4,

			VERT_FORMAT_INT,
			VERT_FORMAT_INT2,
			VERT_FORMAT_INT3,
			VERT_FORMAT_INT4,

			VERT_FORMAT_FLOAT,
			VERT_FORMAT_FLOAT2,
			VERT_FORMAT_FLOAT3,
			VERT_FORMAT_FLOAT4
		};

		struct VertexAttribute
		{
			VertexAttributeType type;
			u32 typeIndex;

			VertexAttributeFormat format;
			bool normalized;
			bool isUnsigned;

			u32 offset;
		};

		class VertexDescription
		{
		public:
			VertexDescription() {}

			virtual const VertexAttribute* GetAttributes() const = 0;
			virtual size_t GetAttributeCount() const = 0;
			virtual size_t GetVertexStride() const = 0;
		protected:
			VertexAttribute* attribs = nullptr;
			size_t attribCount = 0;
			size_t vertexStride = 0;
		};
	}
}
