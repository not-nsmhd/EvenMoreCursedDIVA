#include "D3D11Framebuffer.h"
#include "D3D11Common.h"

namespace Starshine::Rendering::D3D11
{
	D3D11Framebuffer::D3D11Framebuffer(ID3D11Device* device, i32 width, i32 height, GFX::TextureFormat format)
		: Width(width), Height(height), Format(format)
	{
		D3D11_TEXTURE2D_DESC texDesc{};

		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = ConversionTables::DXGITextureFormats[static_cast<size_t>(format)];
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		device->CreateTexture2D(&texDesc, nullptr, &ColorTexture);

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

		device->CreateSamplerState(&samplerDesc, &Sampler);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = texDesc.Format;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		device->CreateShaderResourceView(ColorTexture.Get(), &srvDesc, &ShaderResourceView);

		D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc{};
		rtViewDesc.Format = texDesc.Format;
		rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtViewDesc.Buffer.FirstElement = 0;
		rtViewDesc.Buffer.NumElements = 1;
		rtViewDesc.Texture2D.MipSlice = 0;

		device->CreateRenderTargetView(ColorTexture.Get(), &rtViewDesc, &RenderTargetView);

		device->GetImmediateContext(&DeviceContext);
	}

	D3D11Framebuffer::~D3D11Framebuffer()
	{
		Sampler.Reset();
		ShaderResourceView.Reset();
		RenderTargetView.Reset();
		ColorTexture.Reset();
	}

	i32 D3D11Framebuffer::GetWidth() const
	{
		return Width;
	}

	i32 D3D11Framebuffer::GetHeight() const
	{
		return Height;
	}

	GFX::TextureFormat D3D11Framebuffer::GetFormat() const
	{
		return Format;
	}

	void D3D11Framebuffer::SetDebugName(std::string_view name)
	{
	}
}
