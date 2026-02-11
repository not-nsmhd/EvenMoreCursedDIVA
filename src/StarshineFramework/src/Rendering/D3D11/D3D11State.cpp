#include "D3D11State.h"

namespace Starshine::Rendering::D3D11
{
	namespace ConversionTables
	{
		static constexpr std::array<D3D11_BLEND, EnumCount<BlendFactor>()> D3D11BlendFactor
		{
			D3D11_BLEND::D3D11_BLEND_ZERO,
			D3D11_BLEND::D3D11_BLEND_ONE,

			D3D11_BLEND::D3D11_BLEND_SRC_COLOR,
			D3D11_BLEND::D3D11_BLEND_INV_SRC_COLOR,
			D3D11_BLEND::D3D11_BLEND_DEST_COLOR,
			D3D11_BLEND::D3D11_BLEND_INV_DEST_COLOR,

			D3D11_BLEND::D3D11_BLEND_SRC_ALPHA,
			D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND::D3D11_BLEND_DEST_ALPHA,
			D3D11_BLEND::D3D11_BLEND_INV_DEST_ALPHA
		};

		static constexpr std::array<D3D11_BLEND_OP, EnumCount<BlendOperation>()> D3D11BlendOperation
		{
			D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
			D3D11_BLEND_OP::D3D11_BLEND_OP_SUBTRACT,
			D3D11_BLEND_OP::D3D11_BLEND_OP_REV_SUBTRACT,
			D3D11_BLEND_OP::D3D11_BLEND_OP_MIN,
			D3D11_BLEND_OP::D3D11_BLEND_OP_MAX
		};
	}

	D3D11BlendState::D3D11BlendState(ID3D11Device* device, const BlendStateDesc& desc) : BlendState{ desc }
	{
		D3D11_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = ConversionTables::D3D11BlendFactor[static_cast<size_t>(Desc.SrcColor)];
		blendDesc.RenderTarget[0].DestBlend = ConversionTables::D3D11BlendFactor[static_cast<size_t>(Desc.DstColor)];

		blendDesc.RenderTarget[0].SrcBlendAlpha = ConversionTables::D3D11BlendFactor[static_cast<size_t>(Desc.SrcAlpha)];
		blendDesc.RenderTarget[0].DestBlendAlpha = ConversionTables::D3D11BlendFactor[static_cast<size_t>(Desc.DstAlpha)];

		blendDesc.RenderTarget[0].BlendOp = ConversionTables::D3D11BlendOperation[static_cast<size_t>(Desc.ColorOp)];
		blendDesc.RenderTarget[0].BlendOpAlpha = ConversionTables::D3D11BlendOperation[static_cast<size_t>(Desc.AlphaOp)];
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		device->CreateBlendState(&blendDesc, &BlendStateObject);
	}

	D3D11BlendState::~D3D11BlendState()
	{
		BlendStateObject.Reset();
	}

	void D3D11BlendState::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		BlendStateObject->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());
#endif
	}
}
