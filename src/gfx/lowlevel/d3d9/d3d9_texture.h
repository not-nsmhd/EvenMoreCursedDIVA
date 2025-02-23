#pragma once
#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <d3d9.h>
#include "../texture.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace D3D9
		{
			class Texture_D3D9 : public Texture
			{
			public:
				Texture_D3D9() {}

				const char* GetDebugName() const;
				void SetDebugName(const char* name);
				TextureFormat GetFormat() const;

				u32 GetWidth() const;
				u32 GetHeight() const;
				const IDirect3DTexture9* GetBaseTexture() const;

				bool Initialize(IDirect3DDevice9* device, int width, int height, TextureFormat format, u32 flags);
				void Destroy();

				void Bind(u32 unit) const;

				void SetData(const void* data);
				void SetData(const void* data, u32 x, u32 y, u32 width, u32 height);
			private:
				IDirect3DDevice9* device = nullptr;

				IDirect3DTexture9* texture = nullptr;
				D3DFORMAT pixelFormat = D3DFMT_UNKNOWN;
			};
		}
	}
}
#endif
#endif
