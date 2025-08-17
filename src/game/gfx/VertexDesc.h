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
		VertexDesc(ResourceHandle handle) : Resource(ResourceType::VertexDesc, handle) {}
	};

	namespace Detail
	{
		constexpr VertexAttrib ConstructVertexAttrib(
			VertexAttribType type,
			u32 index,
			VertexAttribFormat format,
			u32 components,
			bool normalized,
			u32 vertexSize,
			u32 attribOffset)
		{
			VertexAttrib attrib = {};

			attrib.Type = type;
			attrib.Index = index;
			attrib.Format = format;
			attrib.Components = components;
			attrib.Normalized = normalized;
			attrib.VertexSize = vertexSize;
			attrib.Offset = attribOffset;

			return attrib;
		}
	}
}
