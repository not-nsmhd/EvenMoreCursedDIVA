#pragma once
#include <d3d9.h>
#include "../texture.h"

namespace GFX::LowLevel::D3D9
{
	class Texture_D3D9 : public Texture
	{
	public:
		Texture_D3D9() = delete;
		Texture_D3D9(LPDIRECT3DDEVICE9 d3dDevice) : device(d3dDevice) {}

		u32 GetWidth() const;
		u32 GetHeight() const;
		TextureFormat GetFormat() const;

		bool Initialize(int width, int height, TextureFormat format, u32 flags);
		void Destroy();

		void Bind(u32 unit);

		void SetData(const void *data);
		void SetData(const void *data, u32 x, u32 y, u32 width, u32 height);
	private:
		LPDIRECT3DDEVICE9 device = NULL;

		LPDIRECT3DTEXTURE9 baseTexture = NULL;
	};
}
