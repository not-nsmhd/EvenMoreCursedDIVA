#include <SDL2/SDL.h>
#include "d3d9_defs.h"
#include "d3d9_texture.h"

namespace GFX::LowLevel::D3D9
{
	namespace GFXtoD3D9
	{
		D3DFORMAT Format[] =
		{
			D3DFORMAT::D3DFMT_A8R8G8B8,	// TEXFORMAT_RGBA8
			D3DFORMAT::D3DFMT_R5G6B5,	// TEXFORMAT_RGB565
			D3DFORMAT::D3DFMT_A1R5G5B5, // TEXFORMAT_RGB5_A1
			D3DFORMAT::D3DFMT_A4R4G4B4,	// TEXFORMAT_RGBA4
			D3DFORMAT::D3DFMT_A8		// TEXFORMAT_R8
		};
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

	bool Texture_D3D9::Initialize(int width, int height, TextureFormat format, u32 flags)
	{
		if (width == 0 || height == 0 || format == TextureFormat::TEXFORMAT_UNKNOWN)
		{
			return false;
		}

		this->width = width;
		this->height = height;
		this->format = format;

		D3DFORMAT surfaceFormat = GFXtoD3D9::Format[static_cast<int>(format)];

		HRESULT result = device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, surfaceFormat, D3DPOOL_DEFAULT, &baseTexture, NULL);
		if (result != D3D_OK)
		{
			LOG_ERROR_ARGS("Failed to create a texture. Error: 0x%X", result);
			return false;
		}

		LOG_INFO_ARGS("Created a new %dx%d texture (address: 0x%X)", width, height, baseTexture);

		return true;
	}

	void Texture_D3D9::Destroy()
	{
		if (baseTexture != NULL)
		{
			baseTexture->Release();
			
			if (nameSet)
			{
				LOG_INFO_ARGS("Destroyed texture \"%s\" (%dx%d)", name, width, height);
			}
			else
			{
				LOG_INFO_ARGS("Destroyed a %dx%d texture (address: 0x%X)", width, height, baseTexture);
			}

			baseTexture = NULL;
		}
	}

	void Texture_D3D9::Bind(u32 unit)
	{
		device->SetTexture(unit, baseTexture);
	}

	void Texture_D3D9::SetData(const void *data)
	{
		if (baseTexture != 0 && data != nullptr)
		{
			RECT surfaceRect = { 0, 0, width, height };
			D3DLOCKED_RECT lockedRect = {};
			baseTexture->LockRect(0, &lockedRect, &surfaceRect, 0);

			SDL_memcpy(lockedRect.pBits, data, lockedRect.Pitch * height);
			baseTexture->UnlockRect(0);
		}
	}

	void Texture_D3D9::SetData(const void *data, u32 x, u32 y, u32 width, u32 height)
	{
		if (baseTexture != 0 && data != nullptr)
		{
			RECT surfaceRect = { 0, 0, width, height };
			D3DLOCKED_RECT lockedRect = {};
			baseTexture->LockRect(0, &lockedRect, &surfaceRect, 0);

			SDL_memcpy(lockedRect.pBits, data, lockedRect.Pitch * height);
			baseTexture->UnlockRect(0);
		}
	}
}
