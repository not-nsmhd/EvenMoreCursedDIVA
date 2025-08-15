#pragma once
#include "Types.h"

namespace Starshine::GFX
{
	using ResourceHandle = u32;
	constexpr u32 InvalidResourceHandle = 0xFFFFFFFF;

	enum class ResourceType
	{
		VertexBuffer,
		IndexBuffer,
		VertexDesc,
		Shader,
		Texture
	};

	struct Resource
	{
	public:
		Resource(ResourceType type) : Type(type) {}

		ResourceHandle Handle = InvalidResourceHandle;
		const ResourceType Type{};
	};
}
