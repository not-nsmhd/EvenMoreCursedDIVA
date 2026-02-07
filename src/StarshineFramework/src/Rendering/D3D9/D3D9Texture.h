#pragma once
#include "Rendering/Texture.h"
#include "D3D9Device.h"
#include <d3d9.h>

namespace Starshine::Rendering::D3D9
{
	struct Texture_D3D9 : public Texture
	{
		Texture_D3D9(IDirect3DDevice9* device, u32 width, u32 height, GFX::TextureFormat format, bool dynamic);
		~Texture_D3D9();

		u32 GetWidth() const;
		u32 GetHeight() const;
		GFX::TextureFormat GetFormat() const;

		void SetData(const void* source, u32 x, u32 y, u32 width, u32 height);

		IDirect3DDevice9* Device{};

		IDirect3DTexture9* GPUTexture{};
		IDirect3DTexture9* StagingTexture{};

		DWORD Filter{ D3DTEXF_LINEAR };
		DWORD WrapMode{ D3DTADDRESS_CLAMP };

		u32 Width{};
		u32 Height{};
		GFX::TextureFormat Format{ GFX::TextureFormat::Unknown };
		bool Dynamic{};
	};
}
