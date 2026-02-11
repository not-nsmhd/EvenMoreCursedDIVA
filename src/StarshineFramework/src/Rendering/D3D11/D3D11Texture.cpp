#include "D3D11Texture.h"

namespace Starshine::Rendering::D3D11
{
	namespace ConversionTables
	{
		static constexpr std::array<DXGI_FORMAT, EnumCount<GFX::TextureFormat>()> DXGITextureFormats
		{
			DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_R8_UNORM,

			DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM
		};
	}

	D3D11Texture::D3D11Texture(ID3D11Device* device, i32 width, i32 height, GFX::TextureFormat format, const void* initialData, bool dynamic)
		: Width(width), Height(height), Format(format), Dynamic(dynamic)
	{
		D3D11_TEXTURE2D_DESC texDesc{};

		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = ConversionTables::DXGITextureFormats[static_cast<size_t>(format)];
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA subresData{};
		subresData.pSysMem = initialData;
		subresData.SysMemPitch = static_cast<UINT>(width * GFX::TexturePixelSizes[static_cast<size_t>(format)]);

		device->CreateTexture2D(&texDesc, &subresData, &BaseTexture);

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

		device->CreateShaderResourceView(BaseTexture.Get(), &srvDesc, &ShaderResourceView);

		device->GetImmediateContext(&DeviceContext);
	}

	D3D11Texture::~D3D11Texture()
	{
		ShaderResourceView.Reset();
		Sampler.Reset();
		BaseTexture.Reset();
	}

	i32 D3D11Texture::GetWidth() const
	{
		return Width;
	}

	i32 D3D11Texture::GetHeight() const
	{
		return Height;
	}

	GFX::TextureFormat D3D11Texture::GetFormat() const
	{
		return Format;
	}

	void D3D11Texture::SetData(const void* source, i32 x, i32 y, i32 width, i32 height)
	{
		if (Dynamic && (x + width) <= Width && (y + height) <= Height)
		{
			D3D11_BOX dstBox{};
			dstBox.left = x;
			dstBox.top = y;
			dstBox.right = x + width;
			dstBox.bottom = y + height;

			UINT pitch = static_cast<UINT>(Width * GFX::TexturePixelSizes[static_cast<size_t>(Format)]);
			DeviceContext->UpdateSubresource(BaseTexture.Get(), 0, &dstBox, source, pitch, 0);
		}
	}

	void D3D11Texture::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		BaseTexture->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());

		std::string temp(name);
		temp.append("_Sampler");
		Sampler->SetPrivateData(WKPDID_D3DDebugObjectName, temp.length(), temp.data());

		temp = name;
		temp.append("_ShaderResourceView");
		ShaderResourceView->SetPrivateData(WKPDID_D3DDebugObjectName, temp.length(), temp.data());
#endif
	}
}
