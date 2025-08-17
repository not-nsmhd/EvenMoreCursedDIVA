#pragma once
#include "Resource.h"

namespace Starshine::GFX
{
	struct VertexBuffer : public Resource
	{
	public:
		VertexBuffer(ResourceHandle handle) : Resource(ResourceType::VertexBuffer, handle) {}

		virtual void SetData(void* source, size_t offset, size_t size) = 0;
	};

	struct IndexBuffer : public Resource
	{
	public:
		IndexBuffer(ResourceHandle handle) : Resource(ResourceType::IndexBuffer, handle) {}

		virtual void SetData(void* source, size_t offset, size_t size) = 0;
	};
}
