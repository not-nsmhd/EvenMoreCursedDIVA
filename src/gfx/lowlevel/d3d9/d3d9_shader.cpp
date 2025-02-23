#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <SDL2/SDL.h>
#include "../../../util/logging.h"
#include "d3d9_shader.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace D3D9
		{
			const char* Shader_D3D9::GetDebugName() const
			{
				return debugName;
			}

			void Shader_D3D9::SetDebugName(const char* name)
			{
				size_t nameLength = SDL_strlen(name);
				nameLength = SDL_min(nameLength, 128);

				debugName = new char[nameLength + 1];
				SDL_memcpy(debugName, name, nameLength);
				debugName[nameLength] = '\0';
			}
			
			IDirect3DVertexShader9* Shader_D3D9::GetVertexShader() const
			{
				return vertShader;
			}
			
			IDirect3DPixelShader9* Shader_D3D9::GetFragmentShader() const
			{
				return fragShader;
			}

			bool Shader_D3D9::Initialize(IDirect3DDevice9* device, const u8* vsSource, size_t vsSourceSize, const u8* fsSource, size_t fsSourceSize)
			{
				if (vsSource == nullptr || vsSourceSize == 0 ||
					fsSource == nullptr || fsSourceSize == 0)
				{
					return false;
				}

				this->device = device;

				HRESULT result = S_OK;
				if ((result = device->CreateVertexShader((const DWORD*)vsSource, &vertShader)) != S_OK)
				{
					Logging::LogError("GFX::D3D9", "Failed to create a vertex shader. Result: 0x%X", result);
					return false;
				}

				if ((result = device->CreatePixelShader((const DWORD*)fsSource, &fragShader)) != S_OK)
				{
					Logging::LogError("GFX::D3D9", "Failed to create a fragment shader. Result: 0x%X", result);
					return false;
				}

				return true;
			}

			void Shader_D3D9::Destroy()
			{
				vertShader->Release();
				fragShader->Release();
			}
			
			void Shader_D3D9::Bind() const
			{
				device->SetVertexShader(vertShader);
				device->SetPixelShader(fragShader);
			}
		}
	}
}
#endif
#endif
