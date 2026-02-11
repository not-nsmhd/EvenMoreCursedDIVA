#include "D3D11Shader.h"

namespace Starshine::Rendering::D3D11
{
	D3D11Shader::D3D11Shader(ID3D11Device* device, D3D11ShaderConstBytecode vsBytecode, D3D11ShaderConstBytecode fsBytecode)
	{
		HRESULT result{ S_OK };
		if ((result = device->CreateVertexShader(vsBytecode.Bytecode, vsBytecode.Size, nullptr, &VertexShader)) != S_OK)
		{
			VertexShader.Reset();
			VertexShader = nullptr;
			return;
		}

		if ((result = device->CreatePixelShader(fsBytecode.Bytecode, fsBytecode.Size, nullptr, &FragmentShader)) != S_OK)
		{
			VertexShader.Reset();
			VertexShader = nullptr;

			FragmentShader.Reset();
			FragmentShader = nullptr;
			return;
		}

		VertexShaderBytecode.Size = vsBytecode.Size;
		VertexShaderBytecode.Bytecode = new u8[vsBytecode.Size];
		std::copy(&vsBytecode.Bytecode[0], &vsBytecode.Bytecode[vsBytecode.Size], VertexShaderBytecode.Bytecode);
	}

	D3D11Shader::~D3D11Shader()
	{
		delete[] VertexShaderBytecode.Bytecode;
	}

	bool D3D11Shader::IsUsable()
	{
		return (VertexShader != nullptr) && (FragmentShader != nullptr);
	}

	void D3D11Shader::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		std::string temp(name);
		temp.append("_VS");

		VertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, temp.length(), temp.data());

		temp = name;
		temp.append("_FS");

		FragmentShader->SetPrivateData(WKPDID_D3DDebugObjectName, temp.length(), temp.data());
#endif
	}
}
