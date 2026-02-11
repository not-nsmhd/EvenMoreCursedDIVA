#pragma once
#include "Rendering/VertexDesc.h"
#include "D3D11Device.h"
#include "D3D11Shader.h"
#include <d3d11.h>
#include <wrl.h>

namespace Starshine::Rendering::D3D11
{
	struct D3D11VertexDesc : public VertexDesc
	{
	public:
		D3D11VertexDesc(ID3D11Device* device, const VertexAttrib* attribs, size_t attribCount, const D3D11ShaderBytecode& vsBytecode);
		~D3D11VertexDesc() override;

		void SetDebugName(std::string_view name);

	public:
		UINT VertexStride{};
		Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout{};
	};
}
