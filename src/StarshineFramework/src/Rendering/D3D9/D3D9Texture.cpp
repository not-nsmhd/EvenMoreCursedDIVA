#include "D3D9Texture.h"
#include "D3D9Common.h"

namespace Starshine::Rendering::D3D9
{
	Texture_D3D9::Texture_D3D9(IDirect3DDevice9* device, u32 width, u32 height, GFX::TextureFormat format, bool dynamic) :
		Device(device), Width(width), Height(height), Format(format), Dynamic(dynamic)
	{
		HRESULT result = Device->CreateTexture(width, height, 1,
			dynamic ? D3DUSAGE_DYNAMIC : 0,
			ConversionTables::D3DTextureDataFormats[static_cast<size_t>(format)],
			D3DPOOL_DEFAULT, &GPUTexture, NULL);

		if (!Dynamic)
		{
			Device->CreateTexture(width, height, 1,
				0,
				ConversionTables::D3DTextureDataFormats[static_cast<size_t>(format)],
				D3DPOOL_SYSTEMMEM, &StagingTexture, NULL);
		}
	}

	Texture_D3D9::~Texture_D3D9()
	{
		GPUTexture->Release();
		if (StagingTexture != nullptr) { StagingTexture->Release(); }
	}

	u32 Texture_D3D9::GetWidth() const
	{
		return Width;
	}

	u32 Texture_D3D9::GetHeight() const
	{
		return Height;
	}

	GFX::TextureFormat Texture_D3D9::GetFormat() const
	{
		return Format;
	}

	void Texture_D3D9::SetData(const void* source, u32 x, u32 y, u32 width, u32 height)
	{
		if (source == nullptr) { return; }
		if (x + width > Width || y + height > Height)
		{
			return;
		}
		
		u32 bytesPerPixel = ConversionTables::D3DBytesPerPixel[static_cast<size_t>(Format)];
		if (Dynamic)
		{
			D3DLOCKED_RECT lockedRect{};
			GPUTexture->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD);
			if (lockedRect.pBits != NULL)
			{
				::memcpy(lockedRect.pBits, source, static_cast<size_t>(width * height * bytesPerPixel));
			}
			GPUTexture->UnlockRect(0);
		}
		else
		{
			D3DLOCKED_RECT lockedRect{};
			StagingTexture->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD);
			if (lockedRect.pBits != NULL)
			{
				::memcpy(lockedRect.pBits, source, static_cast<size_t>(width * height * bytesPerPixel));
			}
			StagingTexture->UnlockRect(0);

			Device->UpdateTexture(StagingTexture, GPUTexture);
		}
	}
}
