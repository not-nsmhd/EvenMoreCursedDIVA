#pragma once
#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <d3d9.h>
#include "../shader.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace D3D9
		{
			class Shader_D3D9 : public Shader
			{
			public:
				Shader_D3D9() {}

				const char* GetDebugName() const;
				void SetDebugName(const char* name);

				IDirect3DVertexShader9* GetVertexShader() const;
				IDirect3DPixelShader9* GetFragmentShader() const;

				bool Initialize(IDirect3DDevice9* device, const u8* vsSource, size_t vsSourceSize, const u8* fsSource, size_t fsSourceSize);
				void Destroy();

				void Bind() const;
			private:
				IDirect3DDevice9* device = nullptr;

				IDirect3DVertexShader9* vertShader = 0;
				IDirect3DPixelShader9* fragShader = 0;
			};
		}
	}
}
#endif
#endif
