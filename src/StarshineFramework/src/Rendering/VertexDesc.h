#pragma once
#include "Rendering/Types.h"
#include "GFX/GPUResource.h"

namespace Starshine::Rendering
{
	struct VertexAttrib
	{
		VertexAttribType Type{};
		u32 Index{};

		VertexAttribFormat Format{};
		u32 Components{};
		bool Normalized{};

		u32 VertexSize{};
		u32 Offset{};
	};

	struct VertexDesc : public GFX::GPUResource, NonCopyable
	{
	public:
		VertexDesc() = default;
	};
}
