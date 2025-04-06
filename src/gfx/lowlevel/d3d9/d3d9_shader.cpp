#include <SDL2/SDL.h>
#include "d3d9_shader.h"
#include "d3d9_defs.h"

namespace GFX::LowLevel::D3D9
{
	bool Shader_D3D9::Initialize(const u8 *vsSource, u32 vsSourceSize, const u8 *fsSource, u32 fsSourceSize)
	{
		if (vsSource == nullptr || vsSourceSize == 0 ||
			fsSource == nullptr || fsSourceSize == 0)
		{
			return false;
		}

		HRESULT result = device->CreateVertexShader((const DWORD*)vsSource, &vertShader);
		if (result != D3D_OK)
		{
			LOG_ERROR_ARGS("Failed to create vertex shader. Error: 0x%X", result);
			return false;
		}

		result = device->CreatePixelShader((const DWORD*)fsSource, &fragShader);
		if (result != D3D_OK)
		{
			LOG_ERROR_ARGS("Failed to create fragment shader. Error: 0x%X", result);
			vertShader->Release();
			return false;
		}

		LOG_INFO_ARGS("Created a new shader program (vertex: 0x%X, fragment: 0x%X)", vertShader, fragShader);
		return true;
	}

	void Shader_D3D9::Destroy()
	{
		vertShader->Release();
		fragShader->Release();
		
		if (nameSet)
		{
			LOG_INFO_ARGS("Shader program \"%s\" has been destroyed", name);
		}
		else
		{
			LOG_INFO_ARGS("Shader program (vertex: 0x%X, fragment: 0x%X) has been destroyed", vertShader, fragShader);
		}
	}

	void Shader_D3D9::Bind()
	{
		device->SetVertexShader(vertShader);
		device->SetPixelShader(fragShader);
	}
}
