#pragma once
#include "Common/Types.h"
#include "GFX/GPUResource.h"

namespace Starshine::Rendering
{
	struct VertexBuffer : public GFX::GPUResource, NonCopyable
	{
	public:
		VertexBuffer() = default;

		virtual void SetData(const void* source, size_t offset, size_t size) = 0;
	};

	struct IndexBuffer : public GFX::GPUResource, NonCopyable
	{
	public:
		IndexBuffer() = default;

		virtual void SetData(const void* source, size_t offset, size_t size) = 0;
	};
}
