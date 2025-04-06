#pragma once
#include <d3d9.h>
#include "../buffers.h"

namespace GFX::LowLevel::D3D9
{
	class Buffer_D3D9 : public Buffer
	{
	public:
		Buffer_D3D9() = delete;
		Buffer_D3D9(LPDIRECT3DDEVICE9 d3dDevice) : device(d3dDevice) {}

		u32 GetSize() const;
		BufferType GetType() const;
		BufferUsage GetUsage() const;
		IndexFormat GetIndexFormat() const;

		LPDIRECT3DVERTEXBUFFER9 GetBaseVertexBuffer() const;
		LPDIRECT3DINDEXBUFFER9 GetBaseIndexBuffer() const;

		bool InitializeVertex(BufferUsage usage, size_t initialSize, const void* initialData);
		bool InitializeIndex(BufferUsage usage, IndexFormat indexFormat, size_t initialSize, const void* initialData);
		void Destroy();

		void Bind(UINT vtxStride);

		void SetData(const void* src, size_t offset, size_t size);

		void* Map(BufferMapping mapping);
		bool Unmap();
	private:
		LPDIRECT3DDEVICE9 device = NULL;
		LPDIRECT3DVERTEXBUFFER9 vertexBuffer = NULL;
		LPDIRECT3DINDEXBUFFER9 indexBuffer = NULL;

		bool mapped = false;
		void* mappedAddress = nullptr;
	};
}
