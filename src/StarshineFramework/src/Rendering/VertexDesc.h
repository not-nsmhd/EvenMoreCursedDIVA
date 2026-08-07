#pragma once
#include "Rendering/Types.h"
#include "Graphics/GPUResource.h"

namespace Starshine::Rendering
{
	enum class VertexAttribType : u8
	{
		Position,
		Color,
		TexCoord,

		Count
	};

	enum class VertexAttribFormat : u8
	{
		Float1,
		Float2,
		Float3,
		Float4,

		UnsignedByte4,
		UnsignedByte4Norm,

		Count
	};

	struct VertexAttrib
	{
		VertexAttribType Type{};
		u32 Index{};

		VertexAttribFormat Format{};

		u32 VertexSize{};
		u32 Offset{};
	};

	struct VertexDesc : public Graphics::GPUResource, NonCopyable
	{
	public:
		VertexDesc() = default;
	};
}
