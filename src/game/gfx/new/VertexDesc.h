#pragma once
#include "Resource.h"

namespace Starshine::GFX
{
	enum class VertexAttribType : u8
	{
		Position,
		Color,
		TexCoord
	};

	enum class VertexAttribFormat : u8
	{
		Byte,
		UnsignedByte,
		Short,
		UnsignedShort,
		Int,
		UnsignedInt,
		Float,

		Count
	};

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

	// NOTE: Backend-specific implementations store attributes in backend's native format
	struct VertexDesc : public Resource
	{
	public:
		VertexDesc() : Resource(ResourceType::VertexDesc) {}
	};
}
