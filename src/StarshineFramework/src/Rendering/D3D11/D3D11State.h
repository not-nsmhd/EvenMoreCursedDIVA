#pragma once
#include "Rendering/State.h"
#include "D3D11Device.h"
#include <d3d11.h>
#include <wrl.h>

namespace Starshine::Rendering::D3D11
{
	struct D3D11BlendState : public BlendState
	{
	public:
		D3D11BlendState(ID3D11Device* device, const BlendStateDesc& desc);
		~D3D11BlendState();

		void SetDebugName(std::string_view name);

	public:
		Microsoft::WRL::ComPtr<ID3D11BlendState> BlendStateObject{};
	};
}
