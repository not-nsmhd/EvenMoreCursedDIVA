#include "D3D9Buffers.h"
#include "D3D9Common.h"

namespace Starshine::Rendering::D3D9
{
	VertexBuffer_D3D9::VertexBuffer_D3D9(IDirect3DDevice9* device, const void* initialData, size_t size, bool dynamic)
		: Device(device), Size(size), Dynamic(dynamic)
	{
		Device->CreateVertexBuffer(static_cast<UINT>(size),
			dynamic ? D3DUSAGE_DYNAMIC : D3DUSAGE_WRITEONLY,
			0, D3DPOOL_MANAGED, &BaseBuffer, NULL);

		if (!dynamic || initialData != nullptr)
		{
			void* bufferData{};
			BaseBuffer->Lock(0, static_cast<UINT>(size), &bufferData, D3DLOCK_DISCARD);
			::memcpy(bufferData, initialData, size);
			BaseBuffer->Unlock();
		}
	}

	VertexBuffer_D3D9::~VertexBuffer_D3D9()
	{
		BaseBuffer->Release();
	}

	void VertexBuffer_D3D9::SetData(const void* source, size_t offset, size_t size)
	{
		if (BaseBuffer != nullptr && Dynamic && source != nullptr && size + offset < Size)
		{
			void* bufferData{};
			BaseBuffer->Lock(static_cast<UINT>(offset), static_cast<UINT>(size), &bufferData, D3DLOCK_DISCARD);
			::memcpy(bufferData, source, size);
			BaseBuffer->Unlock();
		}
	}

	IndexBuffer_D3D9::IndexBuffer_D3D9(IDirect3DDevice9* device, const void* initialData, size_t size, IndexFormat format, bool dynamic)
		: Device(device), Size(size), Format(format), Dynamic(dynamic)
	{
		D3DFORMAT indexFormat = ConversionTables::D3DIndexFormats[static_cast<size_t>(format)];

		Device->CreateIndexBuffer(static_cast<UINT>(size),
			dynamic ? D3DUSAGE_DYNAMIC : D3DUSAGE_WRITEONLY,
			indexFormat, D3DPOOL_MANAGED, &BaseBuffer, NULL);

		if (!dynamic || initialData != nullptr)
		{
			void* bufferData{};
			BaseBuffer->Lock(0, static_cast<UINT>(size), &bufferData, D3DLOCK_DISCARD);
			::memcpy(bufferData, initialData, size);
			BaseBuffer->Unlock();
		}
	}

	IndexBuffer_D3D9::~IndexBuffer_D3D9()
	{
		BaseBuffer->Release();
	}

	void IndexBuffer_D3D9::SetData(const void* source, size_t offset, size_t size)
	{
		if (BaseBuffer != nullptr && Dynamic && source != nullptr && size + offset < Size)
		{
			void* bufferData{};
			BaseBuffer->Lock(static_cast<UINT>(offset), static_cast<UINT>(size), &bufferData, D3DLOCK_DISCARD);
			::memcpy(bufferData, source, size);
			BaseBuffer->Unlock();
		}
	}
}
