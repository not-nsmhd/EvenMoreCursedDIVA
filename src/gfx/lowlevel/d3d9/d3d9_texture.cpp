#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <SDL2/SDL.h>
#include "d3d9_texture.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace GFXtoD3D
		{
			D3DFORMAT PixelFormat[] =
			{
				D3DFMT_A8R8G8B8,	// TEXFORMAT_RGBA8
				D3DFMT_R5G6B5,		// TEXFORMAT_RGB565
				D3DFMT_A1R5G5B5,	// TEXFORMAT_RGB5_A1
				D3DFMT_A4R4G4B4,	// TEXFORMAT_RGBA4
				D3DFMT_A8			// TEXFORMAT_R8
			};
		}

		namespace D3D9
		{
			const char* Texture_D3D9::GetDebugName() const
			{
				return debugName;
			}

			void Texture_D3D9::SetDebugName(const char* name)
			{
				size_t nameLength = SDL_strlen(name);
				nameLength = SDL_min(nameLength, 128);

				debugName = new char[nameLength + 1];
				SDL_memcpy(debugName, name, nameLength);
				debugName[nameLength] = '\0';
			}

			TextureFormat Texture_D3D9::GetFormat() const
			{
				return format;
			}

			u32 Texture_D3D9::GetWidth() const
			{
				return width;
			}

			u32 Texture_D3D9::GetHeight() const
			{
				return height;
			}

			const IDirect3DTexture9* Texture_D3D9::GetBaseTexture() const
			{
				return texture;
			}

			bool Texture_D3D9::Initialize(IDirect3DDevice9* device, int width, int height, TextureFormat format, u32 flags)
			{
				if (width == 0 || height == 0 || format == TextureFormat::TEXFORMAT_UNKNOWN)
				{
					return false;
				}

				this->device = device;
				this->width = width;
				this->height = height;
				this->format = format;

				pixelFormat = GFXtoD3D::PixelFormat[static_cast<int>(format)];

				device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, pixelFormat, D3DPOOL_DEFAULT, &texture, NULL);

				return true;
			}

			void Texture_D3D9::Destroy()
			{
				texture->Release();
			}

			void Texture_D3D9::Bind(u32 unit) const
			{
				device->SetTexture(unit, texture);
			}

			void Texture_D3D9::SetData(const void* data)
			{
				if (texture != nullptr && data != nullptr)
				{
					D3DLOCKED_RECT lockedRect;
					texture->LockRect(0, &lockedRect, NULL, 0);
					SDL_memcpy(lockedRect.pBits, data, lockedRect.Pitch * height);
					texture->UnlockRect(0);
				}
			}

			void Texture_D3D9::SetData(const void* data, u32 x, u32 y, u32 width, u32 height)
			{
				if (texture != nullptr && data != nullptr)
				{
					D3DLOCKED_RECT lockedRect;
					RECT rectToLock = { x, y, width, height };

					texture->LockRect(0, &lockedRect, &rectToLock, 0);
					SDL_memcpy(lockedRect.pBits, data, lockedRect.Pitch * height);
					texture->UnlockRect(0);
				}
			}
		}
	}
}
#endif
#endif
