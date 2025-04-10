#pragma once
#include "../../common/int_types.h"
#include "resource.h"

namespace GFX::LowLevel
{
	enum class VertexAttributeType
	{
		POSITION,
		COLOR,
		TEXCOORD,
		NORMAL
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

	class VertexDescription : public Resource
	{
	public:
		VertexDescription() {}

		virtual const VertexAttribute *GetAttributes() const = 0;
		virtual size_t GetAttributeCount() const = 0;
		virtual size_t GetVertexStride() const = 0;

		virtual void Destroy() = 0;
	protected:
		VertexAttribute *attribs = nullptr;
		size_t attribCount = 0;
		size_t vertexStride = 0;
	};
}
