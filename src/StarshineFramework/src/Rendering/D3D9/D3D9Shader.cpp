#include "D3D9Shader.h"
#include <glm/gtc/type_ptr.hpp>

namespace Starshine::Rendering::D3D9
{
	Shader_D3D9::Shader_D3D9(IDirect3DDevice9* device, const DWORD* vsBytecode, const DWORD* fsBytecode)
		: Device(device)
	{
		Device->CreateVertexShader(vsBytecode, &VertexShader);
		Device->CreatePixelShader(fsBytecode, &FragmentShader);
	}

	Shader_D3D9::~Shader_D3D9()
	{
		VertexShader->Release();
		FragmentShader->Release();
	}

	void Shader_D3D9::SetVertexShaderVariables(u32 index, size_t count, const f32* values)
	{
		if (VertexShader != nullptr && values != nullptr && count > 0)
		{
			Device->SetVertexShader(VertexShader);
			Device->SetVertexShaderConstantF(index, values, count);
		}
	}

	void Shader_D3D9::SetFragmentShaderVariables(u32 index, size_t count, const f32* values)
	{
		if (FragmentShader != nullptr && values != nullptr && count > 0)
		{
			Device->SetPixelShader(FragmentShader);
			Device->SetPixelShaderConstantF(index, values, count);
		}
	}

	void Shader_D3D9::SetVertexShaderMatrix(u32 index, const mat4& matrix)
	{
		if (VertexShader != nullptr)
		{
			Device->SetVertexShader(VertexShader);

			glm::mat4 transposed = glm::transpose(matrix);
			auto valuePtr = glm::value_ptr(transposed);
			Device->SetVertexShaderConstantF(index, valuePtr, 4);
		}
	}

	void Shader_D3D9::SetFragmentShaderMatrix(u32 index, const mat4& matrix)
	{
		if (FragmentShader != nullptr)
		{
			Device->SetPixelShader(FragmentShader);

			glm::mat4 transposed = glm::transpose(matrix);
			auto valuePtr = glm::value_ptr(transposed);
			Device->SetPixelShaderConstantF(index, valuePtr, 4);
		}
	}
}
