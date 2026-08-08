#include "D3D11Texture.h"
#include "D3D11Common.h"
#include <Common/Logging/Logging.h>

namespace Starshine::Rendering::D3D11
{
	D3D11Texture::D3D11Texture(D3D11Device& device, Graphics::Texture* texture)
		: deviceRef(device), ParentTexture(texture)
	{
		const ivec2 texSize = texture->GetSize();
		const Graphics::TextureFormat texFormat = texture->GetFormat();
		const Graphics::TextureFlags texFlags = texture->GetFlags();
		ID3D11Device* baseDev = device.GetBaseDevice();

		D3D11_TEXTURE2D_DESC texDesc{};

		texDesc.Width = texSize.x;
		texDesc.Height = texSize.y;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = ConversionTables::DXGITextureFormats[static_cast<size_t>(texFormat)];
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = texture->GPUTexture.Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA subresData{};
		subresData.pSysMem = texture->GetData();
		subresData.SysMemPitch = static_cast<UINT>(texSize.x * Graphics::TexturePixelSizes[static_cast<size_t>(texFormat)]);

		baseDev->CreateTexture2D(&texDesc, &subresData, &BaseTexture);

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = texFlags.NearestFiltering ? D3D11_FILTER_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = texFlags.WrapS ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = texFlags.WrapT ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

		baseDev->CreateSamplerState(&samplerDesc, &Sampler);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = texDesc.Format;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		baseDev->CreateShaderResourceView(BaseTexture.Get(), &srvDesc, &ShaderResourceView);
	}

	D3D11Texture::~D3D11Texture()
	{
		deviceRef.QueueObjectForDeletion(ShaderResourceView);
	}

	void D3D11Texture::SetData(const void* source, i32 x, i32 y, i32 width, i32 height)
	{
		const bool dynamic = ParentTexture->GPUTexture.Dynamic;
		const ivec2 texSize = ParentTexture->GetSize();

		ID3D11DeviceContext* devContext = {};
		deviceRef.GetBaseDevice()->GetImmediateContext(&devContext);

		if (dynamic && (x + width) <= texSize.x && (y + height) <= texSize.y)
		{
			D3D11_BOX dstBox{};
			dstBox.left = x;
			dstBox.top = y;
			dstBox.right = x + width;
			dstBox.bottom = y + height;

			UINT pitch = static_cast<UINT>(texSize.x * Graphics::TexturePixelSizes[static_cast<size_t>(ParentTexture->GetFormat())]);
			devContext->UpdateSubresource(BaseTexture.Get(), 0, &dstBox, source, pitch, 0);
		}
	}

	void D3D11Texture::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		BaseTexture->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());

		std::string temp = std::string(name);
		temp.append("_ShaderResourceView");
		ShaderResourceView->SetPrivateData(WKPDID_D3DDebugObjectName, temp.length(), temp.data());
#endif
	}
}
