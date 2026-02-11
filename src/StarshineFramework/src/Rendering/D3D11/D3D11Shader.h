#pragma once
#include "Rendering/Shader.h"
#include "D3D11Device.h"
#include <d3d11.h>
#include <wrl.h>

namespace Starshine::Rendering::D3D11
{
	struct D3D11ShaderBytecode
	{
		u8* Bytecode{};
		size_t Size{};
	};

	struct D3D11ShaderConstBytecode
	{
		const u8* Bytecode{};
		const size_t Size{};
	};

	struct D3D11Shader : public Shader
	{
	public:
		D3D11Shader(ID3D11Device* device, D3D11ShaderConstBytecode vsBytecode, D3D11ShaderConstBytecode fsBytecode);
		~D3D11Shader() override;

	public:
		bool IsUsable();
		void SetDebugName(std::string_view name);

	public:
		D3D11ShaderBytecode VertexShaderBytecode{};

		Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader{};
		Microsoft::WRL::ComPtr<ID3D11PixelShader> FragmentShader{};
	};
}
