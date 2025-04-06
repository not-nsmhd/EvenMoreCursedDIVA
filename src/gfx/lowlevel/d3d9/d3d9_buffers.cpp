#include "d3d9_buffers.h"
#include "d3d9_defs.h"

namespace GFX::LowLevel::D3D9
{
	namespace GFXtoD3D9
	{
		DWORD BufferUsage[] = 
		{
			D3DUSAGE_WRITEONLY, // BUFFER_USAGE_STATIC
			D3DUSAGE_DYNAMIC	// BUFFER_USAGE_DYNAMIC
		};

		D3DFORMAT IndexFormat[] = 
		{
			D3DFORMAT::D3DFMT_INDEX16,
			D3DFORMAT::D3DFMT_INDEX32
		};
	}

	u32 Buffer_D3D9::GetSize() const
	{
		return size;
	}

	BufferType Buffer_D3D9::GetType() const
	{
		return type;
	}

	BufferUsage Buffer_D3D9::GetUsage() const
	{
		return usage;
	}

	IndexFormat Buffer_D3D9::GetIndexFormat() const
	{
		return indexFormat;
	}

	LPDIRECT3DVERTEXBUFFER9 Buffer_D3D9::GetBaseVertexBuffer() const
	{
		return vertexBuffer;
	}

	LPDIRECT3DINDEXBUFFER9 Buffer_D3D9::GetBaseIndexBuffer() const
	{
		return indexBuffer;
	}

	bool Buffer_D3D9::InitializeVertex(BufferUsage usage, size_t initialSize, const void *initialData)
	{
		if (usage == BufferUsage::BUFFER_USAGE_STATIC && (initialSize == 0 || initialData == nullptr))
		{
			return false;
		}

		if (initialSize == 0)
		{
			return false;
		}

		this->type = BufferType::BUFFER_VERTEX;
		this->usage = usage;

		DWORD d3dUsage = GFXtoD3D9::BufferUsage[static_cast<i32>(usage)];
		HRESULT result = device->CreateVertexBuffer(initialSize, d3dUsage, 0, D3DPOOL_MANAGED, &vertexBuffer, NULL);

		if (result != D3D_OK)
		{
			LOG_ERROR_ARGS("Failed to create a vertex buffer. Error: 0x%X", result);
			return false;
		}

		size = initialSize;

		if (initialData != nullptr)
		{
			void* bufferData = nullptr;
			vertexBuffer->Lock(0, initialSize, &bufferData, D3DLOCK_DISCARD);
			SDL_memcpy(bufferData, initialData, initialSize);
			vertexBuffer->Unlock();
		}

		LOG_INFO_ARGS("Created a new vertex buffer at address 0x%X", vertexBuffer);
		return true;
	}

	bool Buffer_D3D9::InitializeIndex(BufferUsage usage, IndexFormat indexFormat, size_t initialSize, const void *initialData)
	{
		if (usage == BufferUsage::BUFFER_USAGE_STATIC && (initialSize == 0 || initialData == nullptr))
		{
			return false;
		}

		if (initialSize == 0)
		{
			return false;
		}

		this->type = BufferType::BUFFER_INDEX;
		this->usage = usage;

		DWORD d3dUsage = GFXtoD3D9::BufferUsage[static_cast<i32>(usage)];
		D3DFORMAT d3dFormat = GFXtoD3D9::IndexFormat[static_cast<i32>(indexFormat)];
		HRESULT result = device->CreateIndexBuffer(initialSize, d3dUsage, d3dFormat, D3DPOOL_MANAGED, &indexBuffer, NULL);

		if (result != D3D_OK)
		{
			LOG_ERROR_ARGS("Failed to create an index buffer. Error: 0x%X", result);
			return false;
		}

		size = initialSize;
		this->indexFormat = indexFormat;

		if (initialData != nullptr)
		{
			void* bufferData = nullptr;
			indexBuffer->Lock(0, initialSize, &bufferData, D3DLOCK_DISCARD);
			SDL_memcpy(bufferData, initialData, initialSize);
			indexBuffer->Unlock();
		}

		LOG_INFO_ARGS("Created a new index buffer at address 0x%X", indexBuffer);
		return true;
	}

	void Buffer_D3D9::Destroy()
	{
		void* destroyedBuffer = nullptr;

		if (vertexBuffer != NULL)
		{
			destroyedBuffer = vertexBuffer;
			vertexBuffer->Release();
			vertexBuffer = NULL;
		}
		
		if (indexBuffer != NULL)
		{
			destroyedBuffer = indexBuffer;
			indexBuffer->Release();
			indexBuffer = NULL;
		}
		
		if (nameSet)
		{
			LOG_INFO_ARGS("Buffer \"%s\" has been destroyed", name);
		}
		else
		{
			LOG_INFO_ARGS("Buffer at address 0x%X has been destroyed", destroyedBuffer);
		}
	}

	void Buffer_D3D9::Bind(UINT vtxStride)
	{
		switch (type)
		{
			case BufferType::BUFFER_VERTEX:
				device->SetStreamSource(0, vertexBuffer, 0, vtxStride);
				break;
			case BufferType::BUFFER_INDEX:
				device->SetIndices(indexBuffer);
				break;
		}
	}

	void Buffer_D3D9::SetData(const void *src, size_t offset, size_t size)
	{
		void* bufferData = nullptr;

		switch (type)
		{
			case BufferType::BUFFER_VERTEX:
				vertexBuffer->Lock(offset, size, &bufferData, D3DLOCK_DISCARD);
				SDL_memcpy(bufferData, src, size);
				vertexBuffer->Unlock();
				break;
			case BufferType::BUFFER_INDEX:
				indexBuffer->Lock(offset, size, &bufferData, D3DLOCK_DISCARD);
				SDL_memcpy(bufferData, src, size);
				indexBuffer->Unlock();
				break;
		}
	}

	void *Buffer_D3D9::Map(BufferMapping mapping)
	{
		if (mapped)
		{
			return mappedAddress;
		}

		return nullptr;
	}

	bool Buffer_D3D9::Unmap()
	{
		return false;
	}
}
