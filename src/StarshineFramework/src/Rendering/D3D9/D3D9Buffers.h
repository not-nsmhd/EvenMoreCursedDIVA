#pragma once
#include "Rendering/Buffers.h"
#include "D3D9Device.h"
#include <d3d9.h>

namespace Starshine::Rendering::D3D9
{
	struct VertexBuffer_D3D9 : public VertexBuffer
	{
		VertexBuffer_D3D9(IDirect3DDevice9* device, const void* initialData, size_t size, bool dynamic);
		~VertexBuffer_D3D9();

		void SetData(const void* source, size_t offset, size_t size) override;

		IDirect3DDevice9* Device{};

		IDirect3DVertexBuffer9* BaseBuffer{};
		size_t Size{};
		bool Dynamic{};
	};

	struct IndexBuffer_D3D9 : public IndexBuffer
	{
		IndexBuffer_D3D9(IDirect3DDevice9* device, const void* initialData, size_t size, IndexFormat format, bool dynamic);
		~IndexBuffer_D3D9();

		void SetData(const void* source, size_t offset, size_t size) override;

		IDirect3DDevice9* Device{};

		IDirect3DIndexBuffer9* BaseBuffer{};
		IndexFormat Format{};
		size_t Size{};
		bool Dynamic{};
	};
}
