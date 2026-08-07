#pragma once
#include "Rendering/Texture.h"
#include "D3D11Device.h"
#include <d3d11.h>
#include <wrl.h>

namespace Starshine::Rendering::D3D11
{
	struct D3D11Texture : public Texture
	{
	public:
		D3D11Texture(D3D11Device& device, Graphics::Texture* texture);
		~D3D11Texture() override;

	public:
		void SetData(const void* source, i32 x, i32 y, i32 width, i32 height);
		void SetDebugName(std::string_view name);

	public:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> BaseTexture{};
		Microsoft::WRL::ComPtr<ID3D11SamplerState> Sampler{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView{};
		D3D11Device& deviceRef;

		Graphics::Texture* ParentTexture{};
	};
}
