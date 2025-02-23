#pragma once
#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <d3d9.h>
#include "../buffers.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace D3D9
		{
			class Buffer_D3D9 : public Buffer
			{
			public:
				Buffer_D3D9() {}

				bool Mapped = false;

				u32 GetSize() const;
				BufferType GetType() const;
				BufferUsage GetUsage() const;
				IndexFormat GetIndexFormat() const;

				IDirect3DVertexBuffer9* GetBaseVertexBuffer() const;
				IDirect3DIndexBuffer9* GetBaseIndexBuffer() const;

				bool Initialize(IDirect3DDevice9* device, BufferType type, BufferUsage usage, size_t initialSize, const void* initialData);
				bool Initialize(IDirect3DDevice9* device, BufferType type, BufferUsage usage, IndexFormat indexFormat, size_t initialSize, const void* initialData);
				void Destroy();

				void SetData(const void* src, size_t offset, size_t size);
			private:
				IDirect3DDevice9* device;

				IDirect3DVertexBuffer9* vertexBuffer;
				IDirect3DIndexBuffer9* indexBuffer;
				u32 size = 0;

				BufferType type = BufferType::BUFFER_NONE;
				BufferUsage usage = BufferUsage::BUFFER_USAGE_NONE;
				IndexFormat indexFormat = IndexFormat::INDEX_16BIT;
			};
		}
	}
}
#endif
#endif
