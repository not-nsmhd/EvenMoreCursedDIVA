#pragma once
#include "Common/Types.h"
#include "Graphics/GPUResource.h"

namespace Starshine::Rendering
{
	struct VertexBuffer : public Graphics::GPUResource, NonCopyable
	{
	public:
		VertexBuffer() = default;
		~VertexBuffer() = default;

		virtual void SetData(const void* source, size_t offset, size_t size) = 0;
	};

	struct IndexBuffer : public Graphics::GPUResource, NonCopyable
	{
	public:
		IndexBuffer() = default;
		~IndexBuffer() = default;

		virtual void SetData(const void* source, size_t offset, size_t size) = 0;
	};

	struct UniformBuffer : public Graphics::GPUResource, NonCopyable
	{
	public:
		UniformBuffer() = default;
		~UniformBuffer() = default;

		virtual void SetData(const void* source, size_t offset, size_t size) = 0;
	};
}
