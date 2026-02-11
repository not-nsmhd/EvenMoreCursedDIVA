#include "D3D11VertexDesc.h"
#include <array>
#include "Common/MathExt.h"

namespace Starshine::Rendering::D3D11
{
	namespace ConversionTables
	{
		static constexpr std::array<const char*, EnumCount<VertexAttribType>()> AttribSemanticNames
		{
			"POSITION",
			"COLOR",
			"TEXCOORD"
		};

		static constexpr std::array<DXGI_FORMAT, EnumCount<VertexAttribFormat>()> AttribDXGIFormats
		{
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,

			DXGI_FORMAT_R8G8B8A8_UINT,
			DXGI_FORMAT_R8G8B8A8_UNORM
		};
	}

	D3D11VertexDesc::D3D11VertexDesc(ID3D11Device* device, const VertexAttrib* attribs, size_t attribCount, const D3D11ShaderBytecode& vsBytecode)
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 8> inputElements_temp;

		for (size_t i = 0; i < attribCount; i++)
		{
			const VertexAttrib* stAttrib = &attribs[i];

			inputElements_temp[i].SemanticName = ConversionTables::AttribSemanticNames[static_cast<size_t>(stAttrib->Type)];
			inputElements_temp[i].SemanticIndex = stAttrib->Index;
			inputElements_temp[i].Format = ConversionTables::AttribDXGIFormats[static_cast<size_t>(stAttrib->Format)];
			inputElements_temp[i].InputSlot = 0;
			inputElements_temp[i].AlignedByteOffset = stAttrib->Offset;
			inputElements_temp[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			inputElements_temp[i].InstanceDataStepRate = 0;

			VertexStride = MathExtensions::Max(VertexStride, stAttrib->VertexSize);
		}

		device->CreateInputLayout(inputElements_temp.data(), attribCount, vsBytecode.Bytecode, vsBytecode.Size, &InputLayout);
	}

	D3D11VertexDesc::~D3D11VertexDesc()
	{
		InputLayout.Reset();
	}

	void D3D11VertexDesc::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		InputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());
#endif
	}
}
