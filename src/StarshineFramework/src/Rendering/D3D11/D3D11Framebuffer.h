#pragma once
#include "Rendering/Framebuffer.h"
#include "D3D11Device.h"
#include <d3d11.h>
#include <wrl.h>

namespace Starshine::Rendering::D3D11
{
	struct D3D11Framebuffer : public Framebuffer
	{
	public:
		D3D11Framebuffer(ID3D11Device* device, i32 width, i32 height, GFX::TextureFormat format);
		~D3D11Framebuffer() override;

	public:
		i32 GetWidth() const;
		i32 GetHeight() const;
		GFX::TextureFormat GetFormat() const;

		void SetDebugName(std::string_view name);

	public:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> ColorTexture{};
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView{};
		Microsoft::WRL::ComPtr<ID3D11SamplerState> Sampler{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView{};
		ID3D11DeviceContext* DeviceContext{};

		i32 Width{};
		i32 Height{};
		GFX::TextureFormat Format{};
	};
}
