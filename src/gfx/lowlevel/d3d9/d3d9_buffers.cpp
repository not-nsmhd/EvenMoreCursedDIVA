#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include "d3d9_buffers.h"
#include <SDL2/SDL.h>

namespace GFX
{
	namespace LowLevel
	{
		namespace GFXtoD3D9
		{
			DWORD BufferUsage[] = 
			{
				D3DUSAGE_WRITEONLY,		// BUFFER_USAGE_STATIC
				D3DUSAGE_DYNAMIC		// BUFFER_USAGE_DYNAMIC
			};

			D3DFORMAT IndexFormat[] = 
			{
				D3DFMT_INDEX16,
				D3DFMT_INDEX32
			};
		}

		namespace D3D9
		{
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

			IDirect3DVertexBuffer9* Buffer_D3D9::GetBaseVertexBuffer() const
			{
				return vertexBuffer;
			}

			IDirect3DIndexBuffer9* Buffer_D3D9::GetBaseIndexBuffer() const
			{
				return indexBuffer;
			}

			bool Buffer_D3D9::Initialize(IDirect3DDevice9* device, BufferType type, BufferUsage usage, size_t initialSize, const void* initialData)
			{
				if (usage == BufferUsage::BUFFER_USAGE_STATIC && (initialSize == 0 || initialData == nullptr))
				{
					return false;
				}

				if (initialSize == 0)
				{
					return false;
				}

				this->type = type;
				this->usage = usage;
				this->device = device;

				DWORD bufferUsage = GFXtoD3D9::BufferUsage[static_cast<int>(usage)];
				HRESULT result = 0;

				if ((result = device->CreateVertexBuffer(initialSize, bufferUsage, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL)) != S_OK)
				{
					return false;
				}

				if (initialData != nullptr)
				{
					void* bufferData = nullptr;
					vertexBuffer->Lock(0, initialSize, &bufferData, D3DLOCK_DISCARD);
					SDL_memcpy(bufferData, initialData, initialSize);
					vertexBuffer->Unlock();
				}

				size = initialSize;
				return true;
			}

			bool Buffer_D3D9::Initialize(IDirect3DDevice9* device, BufferType type, BufferUsage usage, IndexFormat indexFormat, size_t initialSize, const void* initialData)
			{
				if (usage == BufferUsage::BUFFER_USAGE_STATIC && (initialSize == 0 || initialData == nullptr))
				{
					return false;
				}

				if (initialSize == 0)
				{
					return false;
				}

				this->type = type;
				this->usage = usage;
				this->device = device;

				DWORD bufferUsage = GFXtoD3D9::BufferUsage[static_cast<int>(usage)];
				HRESULT result = 0;
				D3DFORMAT indexFormat_d3d = GFXtoD3D9::IndexFormat[static_cast<int>(indexFormat)];

				if ((result = device->CreateIndexBuffer(initialSize, bufferUsage, indexFormat_d3d, D3DPOOL_DEFAULT, &indexBuffer, NULL)) != S_OK)
				{
					return false;
				}

				if (initialData != nullptr)
				{
					void* bufferData = nullptr;
					indexBuffer->Lock(0, initialSize, &bufferData, D3DLOCK_DISCARD);
					SDL_memcpy(bufferData, initialData, initialSize);
					indexBuffer->Unlock();
				}

				size = initialSize;
				this->indexFormat = indexFormat;
				return true;
			}

			void Buffer_D3D9::Destroy()
			{
				if (type == BufferType::BUFFER_VERTEX)
				{
					vertexBuffer->Release();
				}
				else if (type == BufferType::BUFFER_INDEX)
				{
					indexBuffer->Release();
				}
			}
			
			void Buffer_D3D9::SetData(const void* src, size_t offset, size_t size)
			{
				if (src == nullptr || device == nullptr)
				{
					return;
				}

				void* bufferData = nullptr;
				if (type == BufferType::BUFFER_VERTEX)
				{
					vertexBuffer->Lock(static_cast<UINT>(offset), static_cast<UINT>(size), &bufferData, D3DLOCK_DISCARD);
					SDL_memcpy(bufferData, src, size);
					vertexBuffer->Unlock();
				}
				else if (type == BufferType::BUFFER_INDEX)
				{
					indexBuffer->Lock(static_cast<UINT>(offset), static_cast<UINT>(size), &bufferData, D3DLOCK_DISCARD);
					SDL_memcpy(bufferData, src, size);
					indexBuffer->Unlock();
				}
			}
		}
	}
}
#endif
#endif